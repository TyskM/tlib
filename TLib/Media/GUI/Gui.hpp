/*   _____                           
 * /\  _  \                     __    
 * \ \ \_\ \      __    __  __ /\_\   
 *  \ \  __ \   /'_ `\ /\ \/\ \\/\ \  
 *   \ \ \/\ \ /\ \_\ \\ \ \_\ \\ \ \ 
 *    \ \_\ \_\\ \____ \\ \____/ \ \_\
 *     \/_/\/_/ \/____\ \\/___/   \/_/
 *                /\____/             
 *                \_/__/              
 *
 * Copyright (c) 2011 Joshua Larouche
 * 
 *
 * License: (BSD)
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Agui nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/Queue.hpp>
#include <TLib/Containers/Stack.hpp>

#include <TLib/Media/GUI/Graphics.hpp>
#include <TLib/Media/GUI/Widget.hpp>
#include <TLib/Media/GUI/EventArgs.hpp>
#include <TLib/Media/GUI/Input.hpp>
#include <TLib/Media/GUI/BaseTypes.hpp>
#include <TLib/Media/GUI/FocusManager.hpp>
#include <TLib/Media/GUI/KeyboardListener.hpp>
#include <TLib/Media/GUI/MouseListener.hpp>
#include <TLib/Media/GUI/CursorProvider.hpp>
#include <TLib/Media/GUI/Transform.hpp>
#include <TLib/Media/GUI/TapListener.hpp>

namespace agui
{
    class AGUI_CORE_DECLSPEC TopContainer;
    class AGUI_CORE_DECLSPEC ToolTip;
    /**
     * @mainpage
     * @section Welcome
     * This is the documentation for the classes in the Agui Library.
     *
     * Agui is a cross platform and back end independent library for Graphical User Interfaces in games. 
     * This means that any code you write with
     * it will work in whatever environment you are working with. At this time, only an Allegro 5
     * back end is developed, but you are free to develop your own.
     *
     * In addition, every class in Agui that uses text uses UTF8. 
     * This means that Unicode is natively supported throughout.
     */

    /**
     * Class for a Gui.
     *
     * You should create one of these for every Gui you need.
     *
     * All desktop Widgets must be added through this.
     *
     * This class requires you to setInput and setGraphics for it to work correctly.
     * @author Joshua Larouche
     * @since 0.1.0
     */
    class AGUI_CORE_DECLSPEC Gui
    {
        #pragma region Members

        FocusManager focusMan;
        double currentTime;
        Vector<KeyboardListener*> keyPreviewListeners;
        Vector<MouseListener*> mousePreviewListeners;
        Vector<TapListener*> tapListeners;
        Input* input;
        Graphics* graphicsContext;
        Stack<Widget*> flaggedWidgets;
        TopContainer* baseWidget;
        double lasttickTime;
        double lastToolTipTime;
        double toolTipShowLength;
        ToolTip* toolTip;
        int maxToolTipWidth;
        bool hasHiddenToolTip;
        Stack<Widget*> q;
        Queue<MouseInput> queuedMouseDown;
        MouseInput emptyMouse;
        MouseEvent relArgs;
        bool destroyingFlaggedWidgets;
        CursorProvider* cursorProvider;
    
        double timerInterval;

        KeyEnum tabNextKey;
        ExtendedKeyEnum tabNextExtKey;
        bool    tabNextShift;
        bool    tabNextControl;
        bool    tabNextAlt;
        KeyEnum tabPreviousKey;
        ExtendedKeyEnum tabPreviousExtKey;
        bool    tabPreviousShift;
        bool    tabPreviousControl;
        bool    tabPreviousAlt;

        //used to focus a tabable widget
        bool passedFocus; 

        bool tabbingEnabled;
        bool focusEnabled;

        //modal variable

        MouseEvent mouseEvent;
        double doubleClickExpireTime;
        double doubleClickInterval;
        Widget* lastMouseDownControl;
        Widget* previousWidgetUnderMouse;
        Widget* widgetUnderMouse;
        Widget* controlWithLock;
        Widget* lastHoveredControl;
        Widget *mouseUpControl;
        bool canDoubleClick;

        double hoverInterval;
        double timeUntilNextHover;

        KeyEvent keyEvent;
        MouseButtonEnum lastMouseButton;

        bool wantWidgetLocationChanged;
        bool enableExistanceCheck;

        bool useTransform;
        Transform transform;

        bool delayMouseDown;
        
        Vector2i lastDragPos;
        Vector2i startDragPos;
        double   downTime;
        double   touchInertia;
        double   lastInertiaTime;
        Widget*  inertiaReceiver;

        Queue<Widget*> frontWidgets;
        Queue<Widget*> backWidgets;

        #pragma endregion

        #pragma region Private Methods

        void haltInertia();
        void processInertia();
        void beginInertia(Widget* target, float speed);

        // Converts the mouse event's position into one that is relative to the parameter widget.
        void makeRelArgs(Widget *source);

        void handleHover();

        void handleDoubleClick();

        // Invalidates the double click event.
        void resetDoubleClickTime();

        // Invalidates the hover event.
        void resetHoverTime();

        void setKeyEvent(const KeyboardInput &keyboard,bool handled);

        void setLastMouseDownControl(Widget* control);

        void setMouseEvent(const MouseInput &mouse);

        void setMouseButtonDown(MouseButtonEnum button);

        Widget* recursiveGetWidgetUnderMouse(Widget* root, const MouseEvent &mouse);

        void handleTimedEvents();

        // Handles the ToolTip hide logic.
        void handleToolTip();

        // @return last mouse down widget.
        Widget* getLastMouseDownControl() const;

        MouseButtonEnum getMouseButtonDown() const;

        MouseEvent getMouseEvent() const;

        bool widgetIsModalChild(Widget* widget) const;

        /* @return True if the focused widget was passed. */
        bool recursiveFocusNext( Widget* target, Widget* focused);

        virtual void focusNextTabableWidget();

        /* @return True if the focused widget was passed. */
        bool recursiveFocusPrevious( Widget* target, Widget* focused);

        virtual void focusPreviousTabableWidget();

        /* Handles mouse move and mouse wheel events.
           If a widget's location, size, or visibility has changed, the Gui will call this
           with isLocationEvent = true. */
        void handleMouseAxes(const MouseInput &mouse,bool isLocationEvent = false);

        void handleMouseDown(const MouseInput &mouse);
        void handleMouseUp(const MouseInput &mouse);
        void handleKeyDown(const KeyboardInput &keyboard);
        void handleKeyUp(const KeyboardInput &keyboard);

        /* Handles a key being held and not released thereby triggering a repeat. */
        void handleKeyRepeat(const KeyboardInput &keyboard);

        // Calls Widget::logic() for every widget starting at base widget.
        void recursiveDoLogic(Widget* baseWidget);

        /* Removes the widget from the Gui. It essentially NULLs all pointers of the parameter widget used by the Gui
           to avoid crashes if a widget was under the mouse at the time of its death. */
        void _removeWidget(Widget *widget);

        bool handleTabbing();

        // @return True if the parameter widget exists.
        bool widgetExists(const Widget* root, const Widget* target) const;

        // @return The focused widget or NULL if none are focused.
        Widget* getFocusedWidget() const;

        /* Dispatches a keyboard event to the listeners. If a listener handles it,
           the focused widget will not receive it. */
        void _dispatchKeyPreview(KeyEvent &keyEvent, KeyEvent::KeyboardEventEnum type);

        /* Dispatches a mouse event to the listeners. If a listener handles it,
           the intended widget will not receive it.
           Only sends basic OS events. Not events like Drag, Enter, Leave,etc */
        void _dispatchMousePreview(const MouseInput& input, MouseEvent::MouseEventEnum type);
        void _dispatchKeyboardEvents();

        void _dispatchMouseEvents();

        #pragma endregion

        #pragma region Public
    public:

        Gui();

        virtual ~Gui(void);

        // Releases widget under mouse
        void _modalChanged();
        
        // @return last found widget under the mouse.
        virtual Widget* getWidgetUnderMouse() const;

        // Calls _removeWidget.
        void _dispatchWidgetDestroyed(Widget* widget);

        // Called by a widget when its location, size, or visibility changes.
        void _widgetLocationChanged();

        // @return True if the Gui is responsible for dequeuing and calling delete on the flagged widgets.
        bool isDestroyingFlaggedWidgets() const;

        // Set to false if the user is responsible for dequeuing and calling delete on the flagged widgets.
        void setDestroyFlaggedWidgets(bool destroying);

        void setTabbingEnabled(bool tabbing);

        bool isTabbingEnabled() const;

        Stack<Widget*>& getFlaggedWidgets();

        // Called by a widget when it wants to be flagged for destruction.
        void flagWidget(Widget *widget);

        void destroyFlaggedWidgets();

        // Sets the tab next key. Default is KEY_TAB.
        virtual void setTabNextKey(KeyEnum key,
            ExtendedKeyEnum extKey = EXT_KEY_NONE,
            bool shift = false,
            bool control = false, 
            bool alt = false);

        // Sets the tab previous key. Default is KEY_TAB + control.
        virtual void setTabPreviousKey(KeyEnum key,
            ExtendedKeyEnum extKey = EXT_KEY_NONE,
            bool shift = false,
            bool control = false,
            bool alt = false);
    
        // Sets how long the mouse must be over a widget without moving for the widget to receive a hover event in seconds.
        void setHoverInterval(double time);

        // @return How long the mouse must be over a widget without moving for the widget to receive a hover event in seconds.
        double getHoverInterval() const;

        // @return How long after a first click will a second click result in a double click event in seconds.
        double getDoubleClickInterval() const;
       
        //Sets how long after a first click will a second click result in a double click event in seconds.
        void setDoubleClickInterval(double time);
    
        // Resizes the Top widget to the size of the display. Needs the graphics context to be set.
        void resizeToDisplay();
   
        // Adds the parameter widget to the desktop if it has no parent.
        void add(Widget* widget);

    
        // Removes the parameter widget from the desktop if it is on the desktop.
        void remove(Widget* widget);

        void clear();

        bool widgetExists(Widget* target);

        /* Should be called every time your game loop updates.
           It will poll the Input, dequeue all queued mouse and keyboard events, and
           call Widget::logic on every widget in the Gui. */
        virtual void logic();

        // Adds a key preview listener. If a key preview listener handles the event, the focused widget will not receive it.
        void addKeyPreviewListener(KeyboardListener* listener);
    
        // Removes a key preview listener. If a key preview listener handles the event, the focused widget will not receive it.
        void removeKeyPreviewListener( KeyboardListener* listener );
    
        // Adds a mouse preview listener. If a mouse preview listener handles the event, the intended widget will not receive it.
        void addMousePreviewListener(MouseListener* listener);
    
        // Removes a mouse preview listener. If a mouse preview listener handles the event, the intended widget will not receive it.
        void removeMousePreviewListener( MouseListener* listener );

        void addTapListener(TapListener* listener);

        void removeTapListener( TapListener* listener );
        
        /* @return The amount of time in seconds the application has been running.
            Useful for timed events. */
        double getElapsedTime() const;

        /* Will paint every widget in the Gui and their children.
           Call this each time you render. */
        void render();

        /* Set the graphics context for the Gui. Will resize the Gui to the display size. */
        void setGraphics(Graphics *context);

        /* Set the input for the Gui. Will resize the Gui to the display size. */
        void setInput(Input* input);

        /* Set the size of the desktop. Call this when your display resizes. */
        void setSize(int width, int height);

        Vector2i getSize() const;

        /* @return The top most, desktop widget of the Gui. Every widget in the Gui is a child of this. */
        TopContainer* getTop() const;

        /* Set the ToolTip used when showing ToolTip text. */
        void setToolTip(ToolTip* toolTip);

        /* @return The ToolTip used if it is set. */
        ToolTip* getToolTip() const;

        /* Shows the ToolTip for the specified Widget. */
        void showToolTip(Widget* widget, int x, int y);

        bool isToolTipVisible() const;

        void hideToolTip();

        /* Set the maximum width used when showing the ToolTip.
           The width can be <= 0 to autosize. If the text is less
           than this maximum, the ToolTip's width will be the minimum
           needed to show the text correctly. */
        void setMaxToolTipWidth(int width);

        int getMaxToolTipWidth() const;

        /* Set whether or not widget exist checks will be made
           when a mouse event occurs. Disable for speedup. */
        void setExistanceCheck(bool check);

        /* @return True if widget exist checks will be made
           when a mouse event occurs. Disable for speedup. */
        int isDoingExistanceCheck() const;

        /* @return The maximum amount of time a ToolTip will show for. */
        double getToolTipShowLength() const;

        /* Set the maximum amount of time a ToolTip will show for. */
        void setToolTipShowLength(double val);

        /* Invalidates the tooltip time. */
        void invalidateToolTip();

        /* The backend specific cursor provider. */
        void setCursorProvider(CursorProvider* provider);

        /* Set the transform to use on the mouse.
           See setUseTransform */
        void setTransform(const Transform& transform);

        /* @return the transform to use on the mouse.
           See setUseTransform */
        const Transform& getTransform() const;

        /* set whether or not to use a transformation on the mouse coordinates. */
        void setUseTransform(bool use);

        /* @return true if the transform is used on the mouse. */
        bool isUsingTransform() const;
        
        /* Set whether or not to use a transformation on the mouse coordinates. */
        void setFocusEnabled(bool enabled);
        
        /* @return true if focus events are enabled. */
        bool isFocusEnabled() const;

        /* Set whether or not the mouse down events are delayed by 1 logic() update.
           Fixes a small issue on touch pads when enabled. */
        void setDelayMouseDownEvents(bool delay);

        /* @return true if the mouse down events are delayed by 1 logic() update. */
        bool isDelayingMouseDownEvents() const;

        /* @return false if the widget under mouse is the top or NULL. */
        bool isWidgetUnderMouse() const;

        /* @return True if the cursor provider is set and it successfully set the cursor */
        bool setCursor(CursorProvider::CursorEnum cursor);

        /* @return The widget with drag focus or NULL */
        Widget* getLockWidget();

        /* Allows you to manually null the Widget under the mouse. */
        void setWidgetUnderMouseToNull();

        /* @return True if widget location changes are tracked accurately. */
        bool isWidgetLocationChangesEnabled() const;

        Input* getInput();
        Graphics* getGraphics();

        /* Teleport the mouse position of this Gui. Forces a logic call. */
        void teleportMouse(int x, int y);

        /* Sets whether or not the expensive call to check if the widget
           under the mouse changed when widgets resize, move etc is called. */
        void toggleWidgetLocationChanged(bool on);


        void bringWidgetToFront(Widget* w);
        void sendWidgetToBack(Widget* w);

        #pragma endregion
    };
}
