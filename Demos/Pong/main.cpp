
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Threading.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Containers/Queue.hpp>
#include <TLib/Containers/Stack.hpp>
#include <enet/enet.h>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>

#include "UI.hpp"

struct PortableBinaryReaderArchive
{
    std::istringstream                 data;
    cereal::PortableBinaryInputArchive read;

    PortableBinaryReaderArchive(const String& _data) : data{_data}, read{data} { }

    PortableBinaryReaderArchive(const uint8_t* ptr, size_t length) :
        data{String(reinterpret_cast<const char*>(ptr), length)}, read{data} { }
};

struct PortableBinaryWriterArchive
{
    std::ostringstream                  data;
    cereal::PortableBinaryOutputArchive write;

    PortableBinaryWriterArchive() : data{}, write{data} { }
};

template<class Archive, typename T>
void save(Archive& ar, const Vector<T>& vec)
{ 
    ar(vec.size());
    for (auto& value : vec)
    { ar(value); }
}

template<class Archive, typename T>
void load(Archive& ar, Vector<T>& vec)
{
    size_t size;
    ar(size);
    vec.resize(size);
    for (size_t i = 0; i < size; i++)
    { ar(vec[i]); }
} 

enum class PacketFlag
{
    Reliable   = ENET_PACKET_FLAG_RELIABLE,
    Unreliable = ENetPacketFlag()
};

template <typename T>
static ENetPacket* createPacket(PacketFlag packetFlags, uint8_t packetType, const T& packet)
{
    PortableBinaryWriterArchive ar;
    ar.write(packetType, packet);
    auto str = ar.data.str();
    return enet_packet_create(str.c_str(), str.length() + 1, (ENetPacketFlag)packetFlags);
}

static void sendPacket(ENetPeer* peer, uint8_t channel, ENetPacket* packet)
{
    enet_peer_send(peer, channel, packet);
}

static void sendPacketToAll(Vector<ENetPeer*> peers, uint8_t channel, ENetPacket* packet)
{
    for (auto& peer : peers)
    { sendPacket(peer, channel, packet); }
}

static PortableBinaryReaderArchive readPacket(ENetPacket* packet)
{
    String str(reinterpret_cast<const char*>(packet->data), packet->dataLength);
    return PortableBinaryReaderArchive(str);
}

Window   window;
FPSLimit fpslimit;

enum class State
{
    Game,
    MainMenu,
    MultiplayerLobby
};

// Assets
Font font;

// Player Input
const int keyMoveUp   = SDL_SCANCODE_W;
const int keyMoveDown = SDL_SCANCODE_S;

const float  fixedTimeStep = 1.f/60.f;
uint16_t     defaultPort   = 6969;

static void loadAssets()
{
    font.loadFromFile("assets/roboto.ttf");
}

enum class PaddleController
{
    None,
    Player1,
    Player2,
    AI
};

struct Paddle
{
    Rectf            rect;
    Vector2f         velocity;
    PaddleController controller = PaddleController::None;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(rect.x, rect.y, rect.width, rect.height);
        ar(velocity.x, velocity.y);
        ar(controller);
    }

    String toString() const
    {
        return fmt::format("Pos: {}", rect.getPos().toString());
    }
};

struct Ball
{
    float    radius = 5.f;
    Vector2f position;
    Vector2f velocity;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(radius);
        ar(position.x, position.y);
        ar(velocity.x, velocity.y);
    }

    String toString() const
    {
        return fmt::format("Rad: {}, Pos: {}, Vel: {}", radius, position.toString(), velocity.toString());
    }
};

struct Game
{
    // Settings
    float    initialBallSpeed                    = 300.f;
    float    paddleSpeed                         = 30.f;
    float    paddleFrictionCoeff                 = 0.92f;
    float    paddleMargin                        = 10.f; // The gap between the paddle and the goal/edge of the screen
    float    paddleWidth                         = 20.f;
    float    paddleHeight                        = 150.f;
    float    ballRadius                          = 8.f;
    float    ballVelocityTransferYCoeff          = 0.33f;
    float    ballSpeedOnPaddleCollisionGainCoeff = 1.10f;
    Vector2f playArea                            = Vector2f(1280, 720);

    // State
    uint32_t       player1Score  = 0;
    uint32_t       player2Score = 0;
    Vector<Paddle> paddles;
    Vector<Ball>   balls;

    // Events
    enum class Events
    {
        Player1Scored,
        Player2Scored
    };
    Queue<Events> events;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(initialBallSpeed,
           paddleSpeed,
           paddleFrictionCoeff,
           paddleMargin,
           paddleWidth, paddleHeight,
           ballRadius,
           ballVelocityTransferYCoeff,
           ballSpeedOnPaddleCollisionGainCoeff,
           playArea.x, playArea.y,
           player1Score, player2Score);
        ar(paddles, balls);
    }

    String toString() const
    {
        return fmt::format("Score: {}/{}, Paddles: {}, Balls: {}", player1Score, player2Score, paddles.toString(), balls.toString());
    }

    void init()
    {
        const float paddleInitialY = playArea.y/2 - paddleHeight/2;

        auto& paddle1      = paddles.emplace_back();
        paddle1.controller = PaddleController::Player1;
        paddle1.rect       = Rectf(paddleMargin, paddleInitialY, paddleWidth, paddleHeight);

        auto& paddle2 = paddles.emplace_back();
        paddle2.controller = PaddleController::Player2;
        paddle2.rect  = Rectf(playArea.x - paddleWidth - paddleMargin, paddleInitialY, paddleWidth, paddleHeight);

        spawnBall();
    }

    void spawnBall()
    {
        auto& ball    = balls.emplace_back();
        ball.position = playArea / 2.f;
        ball.velocity = Vector2f(-initialBallSpeed, 0.f);
        ball.radius   = ballRadius;
    }

    void fixedUpdate(float delta)
    {
        //Vector2f mouseLocalPos = Vector2f(Input::mousePos);
        //Vector2f mouseWorldPos = localToWorldPoint(mouseLocalPos, Renderer2D::getView(), Renderer::getFramebufferSize());
    
        for (auto& paddle : paddles)
        {
            paddle.rect.x   += paddle.velocity.x * delta;
            paddle.rect.y   += paddle.velocity.y * delta;
            paddle.velocity *= paddleFrictionCoeff;

            if (paddle.controller == PaddleController::AI)
            {
                int dir = math::sign(balls[0].position.y - paddle.rect.center().y);
                paddle.velocity.y += dir * paddleSpeed;
            }

            paddle.rect.y = std::clamp(paddle.rect.y, 0.f, playArea.y - paddle.rect.height);
    }

        // Update balls in reverse, bc we might have to remove them while looping
        for (auto i = balls.size(); i--;)
        {
            auto& ball = balls[i];
            Vector2f ballNextFramePos = ball.position + ball.velocity * delta;

            bool ballCollidingTop    = ballNextFramePos.y + ball.radius > playArea.y;
            bool ballCollidingBottom = ballNextFramePos.y - ball.radius < 0;
            if (ballCollidingTop || ballCollidingBottom)
            {
                ball.velocity = ball.velocity.reflect(Vector2f(1.f, 0.f));
            }

            bool ballCollidingLeft  = ballNextFramePos.x - ball.radius < 0;
            bool ballCollidingRight = ballNextFramePos.x + ball.radius > playArea.x;
            if (ballCollidingLeft || ballCollidingRight)
            {
                if (ballCollidingLeft)
                {
                    tlog::info("Player 1 Scored");
                    player1Score += 1;
                    events.push(Events::Player1Scored);
                }
                else
                {
                    tlog::info("Player 2 Scored");
                    player2Score += 1;
                    events.push(Events::Player2Scored);
                }

                std::swap(balls[i], balls.back()); balls.pop_back();
                spawnBall();
                continue;
            }

            for (auto& paddle : paddles)
            {
                if (paddle.rect.intersects(Circlef(ballNextFramePos.x, ballNextFramePos.y, ball.radius)))
                {
                    ball.velocity    = ball.velocity.reflect(Vector2f(0.f, 1.f));
                    ball.velocity.y += paddle.velocity.y * ballVelocityTransferYCoeff;
                    ball.velocity.x *= ballSpeedOnPaddleCollisionGainCoeff;
                }
            }

            ball.position += ball.velocity * delta;
        }
    }

    void draw(float delta)
    {
        const Vector2f viewportCenter = playArea/2.f;

        String scoreLeftStr  = std::to_string(player1Score);
        String scoreRightStr = std::to_string(player2Score);

        Renderer2D::drawLine(Vector2f(viewportCenter.x, 0.f), Vector2f(viewportCenter.x, playArea.y), ColorRGBAf::white());
        Renderer2D::drawText(scoreLeftStr,  font, Vector2f(viewportCenter.x - 20.f - font.calcTextSize(scoreLeftStr).x, viewportCenter.y));
        Renderer2D::drawText(scoreRightStr, font, Vector2f(viewportCenter.x + 20.f,                                     viewportCenter.y));

        for (auto& paddle : paddles)
        {
            Renderer2D::drawRect(paddle.rect, 0.f, true, ColorRGBAf::white());
        }
        for (auto& ball : balls)
        {
            Renderer2D::drawCircle(ball.position, ball.radius, true, ColorRGBAf::white());
        }
    }
};

#pragma region Packets

enum class PacketType : uint8_t
{
    ServerTime,
    PaddleData,
    BallData,
    Score,
    GameState,
    AssignPaddle
};

struct PaddleDataPacket
{
    int32_t paddleIndex = 0;
    Paddle  paddle;

    template <class Archive>
    void serialize(Archive& ar)
    { ar(paddleIndex, paddle); }
};

struct PaddleIDPacket
{
    int32_t paddleIndex = 0;

    template <class Archive>
    void serialize(Archive& ar)
    { ar(paddleIndex); }
};

struct ScorePacket
{
    uint32_t player1Score = 0;
    uint32_t player2Score = 0;

    template <class Archive>
    void serialize(Archive& ar)
    { ar(player1Score, player2Score); }
};

#pragma endregion

struct Server
{
    UPtr<Thread>      serverThread;
    Atomic<bool>      serverRunning = false;
    ENetHost*         host          = nullptr;
    ENetAddress       serverAddress;
    Vector<ENetPeer*> peers;

    Queue<ENetPacket*> fakeLagRecvQueue;
    Queue<ENetPacket*> fakeLagSendQueue;
    float fakeLagRtt = 100.f;

    void reset()
    {
        serverRunning = false;
        if (serverThread) { serverThread->join(); }
        if (host)         { enet_host_destroy(host); }
    }

    void start()
    {
        serverAddress.host = ENET_HOST_ANY;
        serverAddress.port = defaultPort;

        host = enet_host_create(
                             &serverAddress /* the address to bind the server host to */, 
                             2              /* allow up to 32 clients and/or outgoing connections */,
                             2              /* allow up to 2 channels to be used, 0 and 1 */,
                             0              /* assume any amount of incoming bandwidth */,
                             0              /* assume any amount of outgoing bandwidth */);

        ASSERTMSG(host, "Failed to create ENet Server");

        serverThread = makeUnique<Thread>(&Server::loop, this);
    }

    void loop()
    {
        tlog::info("Server: Server Started");

        ENetEvent event;
        Timer     deltaTimer;
        Game      game;
        game.init();

        double time           = 0;
        double lastUpdateTime = 0;
        double timeBuffer     = 0;
        size_t loops          = 0;

        serverRunning = true;
        while (serverRunning)
        {
            enet_host_service(host, &event, 16);
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                tlog::info("Server: A new client connected from {}:{}.\n", 
                        event.peer -> address.host,
                        event.peer -> address.port);
 
                /* Store any relevant client information here. */
                peers.push_back(event.peer);

                { // Send game state
                    tlog::info("Server: Sending game state: {}", game.toString());
                    std::ostringstream ss;
                    {
                        cereal::PortableBinaryOutputArchive ar(ss);
                        ar(PacketType::GameState, game);
                    }
                    ENetPacket* packet = enet_packet_create(ss.str().c_str(), ss.str().length() + 1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                }

                { // Assign paddle
                    std::ostringstream ss;
                    {
                        PaddleIDPacket pidp;
                        pidp.paddleIndex = peers.size() - 1;
                        cereal::PortableBinaryOutputArchive ar(ss);
                        ar(PacketType::AssignPaddle, pidp);
                    }
                    ENetPacket* packet = enet_packet_create(ss.str().c_str(), ss.str().length()+1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                }

                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
            {
                auto ar = readPacket(event.packet);
                PacketType type;
                ar.read(type);
                if (type == PacketType::PaddleData)
                {
                    PaddleDataPacket paddlePkt;
                    ar.read(paddlePkt);
                    game.paddles[paddlePkt.paddleIndex] = paddlePkt.paddle;
                }
                else
                { tlog::error("Client: Got unknown packet type"); }

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy (event.packet);
            
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                tlog::info("Server: Client disconnected.\n");
                /* Reset the peer's client information. */
                event.peer -> data = NULL;
                enet_peer_reset(event.peer);
                eastl::remove_if(peers.begin(), peers.end(), [&](auto& p){ return p == event.peer; });
            }
            }

            // Send server time
            {
                auto serverTimePkt = createPacket(PacketFlag::Unreliable, (uint8_t)PacketType::ServerTime, time);
                sendPacketToAll(peers, 0, serverTimePkt);
            }

            if ((int32_t)loops % 100 == 0)
            {
                tlog::info("Server: Time = {}", time);

                int32_t peeri = 0;
                for (auto& peer : peers)
                {
                    tlog::info("Server: Peer {} RTT = {}", peeri, peer->roundTripTime);
                    ++peeri;
                }
            }

            bool playersReady = peers.size() >= 2;

            // Fixed update
            const double delta = deltaTimer.restart().asSeconds();
            time          += delta;
            timeBuffer    += time - lastUpdateTime;
            lastUpdateTime = time;
            while (timeBuffer >= fixedTimeStep)
            {
                //if (playersReady)
                game.fixedUpdate(fixedTimeStep);
                timeBuffer -= fixedTimeStep;
            }

            while (!game.events.empty())
            {
                auto event = game.events.front();
                game.events.pop();
                switch (event)
                {
                case Game::Events::Player1Scored:
                case Game::Events::Player2Scored:
                {
                    // Send new scores
                    {
                        ScorePacket pktData;
                        pktData.player1Score = game.player1Score;
                        pktData.player2Score = game.player2Score;
                        auto pkt = createPacket(PacketFlag::Reliable, uint8_t(PacketType::Score), pktData);
                        for (auto& peer : peers)
                        { sendPacket(peer, 1, pkt); }
                    }

                    break;
                }
                default: break;
                }

            }

            if (playersReady)
            {
                // Send Player 1 Data to Player 2
                {
                    PaddleDataPacket data;
                    data.paddleIndex = 0;
                    data.paddle      = game.paddles[0];
                    ENetPacket* packet = createPacket(PacketFlag::Unreliable, (uint8_t)PacketType::PaddleData, data);
                    sendPacket(peers[1], 0, packet);
                }

                // Send Player 2 Data to Player 1
                {
                    PaddleDataPacket data;
                    data.paddleIndex = 1;
                    data.paddle      = game.paddles[1];
                    ENetPacket* packet = createPacket(PacketFlag::Unreliable, (uint8_t)PacketType::PaddleData, data);
                    sendPacket(peers[0], 0, packet);
                }
            }

            // Send ball positions to both clients
            {
                auto pkt = createPacket(PacketFlag::Unreliable, uint8_t(PacketType::BallData), game.balls);
                sendPacketToAll(peers, 0, pkt);
            }

            loops++;
        }
    }
};

struct Client
{
    Stack<State> stateStack = { State::Game };
    Game         game;
    ENetHost*    host                = nullptr;
    ENetPeer*    server              = nullptr;
    int32_t      paddleIndex         = 0;
    double       serverTime          = 0;
    double       lastFixedUpdateTime = 0;

    void reset()
    {
        if (host) { enet_host_destroy(host); }
    }

    void connect(const String& ip = "localhost")
    {
        host = enet_host_create (
        NULL /* create a client host */,
        1    /* only allow 1 outgoing connection */,
        2    /* allow up 2 channels to be used, 0 and 1 */,
        0    /* assume any amount of incoming bandwidth */,
        0    /* assume any amount of outgoing bandwidth */);
        ASSERTMSG(host, "Client: Failed to create ENet client host.");

        loadAssets();
        game.init();

        ENetAddress address;
        enet_address_set_host(&address, ip.c_str());
        address.port = defaultPort;

        server = enet_host_connect(host, &address, 2, 0);
        ASSERT(server);

        ENetEvent event;

        /* Wait up to 5 seconds for the connection attempt to succeed. */
        if (enet_host_service (host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            tlog::info("Client: Connection to {}:{} succeeded.", ip, defaultPort);
            enet_host_flush(host);
        }
        else
        {
            /* Either the 5 seconds are up or a disconnect event was */
            /* received. Reset the peer in the event the 5 seconds   */
            /* had run out without any significant event.            */
            enet_peer_reset (server);
            server = nullptr;
            tlog::info("Client: Connection to {}:{} failed.", ip, defaultPort);
        }
    }

    void fixedUpdate(float delta)
    {
        if (!server) { return; }

        int dir = Input::isKeyPressed(keyMoveUp) - Input::isKeyPressed(keyMoveDown);
        game.paddles[paddleIndex].velocity.y += dir * game.paddleSpeed;

        PaddleDataPacket paddlePktData;
        paddlePktData.paddleIndex = paddleIndex;
        paddlePktData.paddle      = game.paddles[paddleIndex];
        auto pkt = createPacket(PacketFlag::Unreliable, (uint8_t)PacketType::PaddleData, paddlePktData);
        sendPacket(server, 0, pkt);

        ENetEvent event;
        while (enet_host_service(host, &event, 0) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    PortableBinaryReaderArchive archive(event.packet->data, event.packet->dataLength);
                    {
                        PacketType type;
                        archive.read(type);

                        if (type == PacketType::ServerTime)
                        {
                            double rttSeconds = server->roundTripTime / 1000.f;
                            archive.read(serverTime);
                            serverTime += rttSeconds;
                        }
                        else if (type == PacketType::BallData)
                        {
                            Vector<Ball> networkBalls;
                            archive.read(networkBalls);
                            game.balls.resize(networkBalls.size());
                            for (size_t i = 0; i < game.balls.size(); i++)
                            {
                                auto& clientBall  = game.balls[i];
                                auto& networkBall = networkBalls[i];

                                Vector2f estimatedPosition = networkBall.position + (networkBall.velocity * delta) * float(serverTime - lastFixedUpdateTime);
                                Vector2f positionError     = estimatedPosition - clientBall.position;
                                const float positionErrorThreshold = 10.f;
                                if (positionError.length() > positionErrorThreshold)
                                {
                                    tlog::info("Client ball position went over error threshold. \n\t Error: {} Estimated: {} Real: {}",
                                        positionError.toString(), estimatedPosition.toString(), clientBall.position.toString());
                                    clientBall.position = clientBall.position.lerp(estimatedPosition, delta * 10.f);
                                }

                                clientBall.velocity = networkBall.velocity;
                            }
                        }
                        else if (type == PacketType::PaddleData)
                        {
                            PaddleDataPacket pkt;
                            archive.read(pkt);
                            game.paddles[pkt.paddleIndex] = pkt.paddle;
                        }
                        else if (type == PacketType::Score)
                        {
                            ScorePacket pkt;
                            archive.read(pkt);
                            game.player1Score = pkt.player1Score;
                            game.player2Score = pkt.player2Score;
                        }
                        else if (type == PacketType::GameState)
                        {
                            archive.read(game);
                        }
                        else if (type == PacketType::AssignPaddle)
                        {
                            PaddleIDPacket pidp;
                            archive.read(pidp);
                            paddleIndex = pidp.paddleIndex;
                        }
                        else
                        {
                            tlog::error("Client: Got unknown packet type: {}", (uint8_t)type);
                        }
                    }
                    enet_packet_destroy(event.packet);
                }

                default: break;
            }
        }

        // TODO: you stopped here
        switch (stateStack.top())
        {
            default: break;
        }

        game.fixedUpdate(delta);

        lastFixedUpdateTime += delta;
    }

    void draw(float delta)
    {
        game.draw(delta);
        beginDiagWidgetExt();
        ImGui::Text("Time: %f", serverTime);
        ImGui::End();
    }
};

Server server;
Client client;

static void init()
{
    if (enet_initialize () != 0)
    {
        tlog::error("An error occurred while initializing ENet.");
        ASSERT(false)
    }

    tlog::info("1: Host Game, 2: Join Game");
    String input;
    std::getline(std::cin, input);
    if (input == "1")
    {
        server.start();
        client.connect("localhost");
    }
    else if (input == "2")
    {
        tlog::info("Enter Server IP:");
        String ip;
        std::getline(std::cin, ip);
        client.connect(ip);
    }
    else
    { tlog::error("the options are 1 and 2, try again u stupid idiot."); abort(); }

}

static void shutdown()
{
    enet_deinitialize();
}

static void fixedUpdate(float delta)
{
    client.fixedUpdate(delta);
}

static void update(float delta)
{
    // Fixed update
    const  float fixedTimeStep  = 1.f/60.f;
    static float time           = 0;
    static float lastUpdateTime = 0;
    static float timeBuffer     = 0;
    time += delta;
    timeBuffer += time - lastUpdateTime;
    lastUpdateTime = time;
    while (timeBuffer >= fixedTimeStep)
    {
        fixedUpdate(fixedTimeStep);
        timeBuffer -= fixedTimeStep;
    }
}

static void draw(float delta)
{
    client.draw(delta);
}

int main()
{
    MyGui imgui;
    Timer deltaTimer;

    WindowCreateParams params;
    params.title = "Window";
    params.size  = {1280, 720};
    window.create(params);
    Input::init(window);
    Renderer::create();
    Renderer2D::create();

    imgui       .create(window);
    deltaTimer  .restart();
    fpslimit    .setFPSLimit(144);
    fpslimit    .setEnabled(true);

    init();

    bool running = true;
    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                // No viewport resizing
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        imgui.newFrame();
        update(delta);
        Renderer::clearColor();
        draw(delta);
        Renderer2D::render();

        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    shutdown();

    return 0;
}
