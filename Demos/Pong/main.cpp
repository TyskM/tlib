
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>

Window   window;
FPSLimit fpslimit;

struct Paddle
{
    Rectf    rect;
    Vector2f velocity;
    bool     ai = false;
};

struct Ball
{
    float    radius = 5.f;
    Vector2f position;
    Vector2f velocity;
};

struct Game
{
    uint64_t       playerLeftScore  = 0;
    uint64_t       playerRightScore = 0;
    Vector<Paddle> paddles;
    Vector<Ball>   balls;
};

Game game;
Font font;

const float initialBallSpeed                    = 300.f;
const float paddleSpeed                         = 30.f;
const float paddleFrictionCoeff                 = 0.92f;
const float paddleMargin                        = 10.f; // The gap between the paddle and the goal/edge of the screen
const float paddleWidth                         = 20.f;
const float paddleHeight                        = 150.f;
const float ballRadius                          = 8.f;
const float ballVelocityTransferYCoeff          = 0.33f;
const float ballSpeedOnPaddleCollisionGainCoeff = 1.10f;

static void spawnBall()
{
    const Vector2f viewportSize = Renderer2D::getView().size;
    auto& ball    = game.balls.emplace_back();
    ball.position = viewportSize / 2.f;
    ball.velocity = Vector2f(-initialBallSpeed, 0.f);
    ball.radius   = ballRadius;
}

static void init()
{
    font.loadFromFile("assets/roboto.ttf");

    const Vector2f viewportSize   = Renderer2D::getView().size;
    const float    paddleInitialY = viewportSize.y/2 - paddleHeight/2;

    auto& playerPaddle = game.paddles.emplace_back();
    playerPaddle.ai    = false;
    playerPaddle.rect  = Rectf(paddleMargin, paddleInitialY, paddleWidth, paddleHeight);

    auto& aiPaddle = game.paddles.emplace_back();
    aiPaddle.ai    = true;
    aiPaddle.rect  = Rectf(viewportSize.x - paddleWidth - paddleMargin, paddleInitialY, paddleWidth, paddleHeight);

    spawnBall();
}

static void shutdown()
{

}

static void fixedUpdate(float delta)
{
    const Vector2f viewportSize = Renderer2D::getView().size;

    Vector2f mouseLocalPos = Vector2f(Input::mousePos);
    Vector2f mouseWorldPos = localToWorldPoint(mouseLocalPos, Renderer2D::getView(), Renderer::getFramebufferSize());

    // Player Input
    const int keyMoveUp   = SDL_SCANCODE_W;
    const int keyMoveDown = SDL_SCANCODE_S;
    
    for (auto& paddle : game.paddles)
    {
        paddle.rect.x   += paddle.velocity.x * delta;
        paddle.rect.y   += paddle.velocity.y * delta;
        paddle.velocity *= paddleFrictionCoeff;

        if (paddle.ai)
        {
            int dir = math::sign(game.balls[0].position.y - paddle.rect.center().y);
            paddle.velocity.y += dir * paddleSpeed;
        }
        else
        {
            int dir = Input::isKeyPressed(keyMoveUp) - Input::isKeyPressed(keyMoveDown);
            paddle.velocity.y += dir * paddleSpeed;
        }

        paddle.rect.y = std::clamp(paddle.rect.y, 0.f, viewportSize.y - paddle.rect.height);
}

    for (auto i = game.balls.size(); i--;)
    {
        auto& ball = game.balls[i];
        Vector2f ballNextFramePos = ball.position + ball.velocity * delta;

        bool ballCollidingTop    = ballNextFramePos.y + ball.radius > viewportSize.y;
        bool ballCollidingBottom = ballNextFramePos.y - ball.radius < 0;
        if (ballCollidingTop || ballCollidingBottom)
        {
            ball.velocity = ball.velocity.reflect(Vector2f(1.f, 0.f));
        }

        bool ballCollidingLeft  = ballNextFramePos.x - ball.radius < 0;
        bool ballCollidingRight = ballNextFramePos.x + ball.radius > viewportSize.x;
        //if (ballCollidingLeft || ballCollidingRight)
        //{
        //    ball.velocity = ball.velocity.reflect(Vector2f(0.f, 1.f));
        //}
        if (ballCollidingLeft || ballCollidingRight)
        {
            std::swap(game.balls[i], game.balls.back()); game.balls.pop_back();

            game.playerLeftScore  += ballCollidingRight;
            game.playerRightScore += ballCollidingLeft;

            spawnBall();

            continue;
        }

        for (auto& paddle : game.paddles)
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
    const Vector2f viewportSize   = Renderer2D::getView().size;
    const Vector2f viewportCenter = viewportSize/2;

    String scoreLeftStr  = std::to_string(game.playerLeftScore);
    String scoreRightStr = std::to_string(game.playerRightScore);

    Renderer2D::drawLine(Vector2f(viewportCenter.x, 0.f), Vector2f(viewportCenter.x, viewportSize.y), ColorRGBAf::white());
    Renderer2D::drawText(scoreLeftStr,  font, Vector2f(viewportCenter.x - 20.f - font.calcTextSize(scoreLeftStr).x, viewportCenter.y));
    Renderer2D::drawText(scoreRightStr, font, Vector2f(viewportCenter.x + 20.f,                                     viewportCenter.y));

    for (auto& paddle : game.paddles)
    {
        Renderer2D::drawRect(paddle.rect, 0.f, true, ColorRGBAf::white());
    }
    for (auto& ball : game.balls)
    {
        Renderer2D::drawCircle(ball.position, ball.radius, true, ColorRGBAf::white());
    }
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
