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

#include <TLib/Media/GUI/Widgets/CheckBox/CheckBox.hpp>
#include <TLib/Media/GUI/Widgets/CheckBox/CheckBoxListener.hpp>
namespace agui {
    CheckBox::CheckBox()
    :	sidePadding(6), autosizingCheckbox(false), checkBoxState(DEFAULT),
        checkedState(UNCHECKED), mouseIsInside(false), mouseIsDown(false),
        isDoingKeyAction(false)
    {
        setMargins(1,1,1,1);
        setBackColor(Color(240,240,240));
        setFocusable(true);
        setTabable(true);
        setCheckBoxAlignment(ALIGN_TOP_LEFT);
        setTextAlignment(ALIGN_MIDDLE_LEFT);

        positionCheckBox();
        resizeCaption();
        setCheckBoxSize(Vector2i(10,10));
    }

    CheckBox::~CheckBox(void)
    {
        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it))
                (*it)->death(this);
        }
    }

    void CheckBox::setCheckBoxSize( const Vector2i &size )
    {
        checkBoxSize = size;
        positionCheckBox();
        if(isAutosizing())
        {
            resizeToContents();
        }
    }

    const Vector2i& CheckBox::getCheckBoxSize() const
    {
        return checkBoxSize;
    }

    void CheckBox::changeCheckBoxState( CheckBoxStateEnum state )
    {
        checkBoxState = state;

        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it))
                (*it)->checkBoxStateChanged(this,state);
        }
    }

    void CheckBox::modifyCheckBoxState()
    {
        if(isDoingKeyAction)
        {
            return;
        }
        if(mouseIsDown && mouseIsInside)
        {
            changeCheckBoxState(CLICKED);
        }
        else if(mouseIsDown && !mouseIsInside)
        {
            changeCheckBoxState(HOVERED);
        }
        else if(!mouseIsDown && mouseIsInside)
        {
            changeCheckBoxState(HOVERED);
        }
        else
        {
            changeCheckBoxState(DEFAULT);
        }
    }

    void CheckBox::nextCheckState()
    {
        switch (getCheckedState())
        {
            case UNCHECKED:
                changeCheckedState(CHECKED);
                break;
            case CHECKED:
            case INTERMEDIATE:
                changeCheckedState(UNCHECKED);
                break;
            default:
                changeCheckedState(CHECKED);
                break;
        }
    }

    CheckBox::CheckBoxStateEnum CheckBox::getCheckBoxState() const
    {
        return checkBoxState;
    }

    CheckBox::CheckBoxCheckedEnum CheckBox::getCheckedState() const
    {
        return checkedState;
    }

    void CheckBox::changeCheckedState( CheckBoxCheckedEnum state )
    {
        checkedState = state;

        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it))
                (*it)->checkedStateChanged(this,state);
        }

        dispatchActionEvent(ActionEvent(this));
    }

    void CheckBox::setIntermediateState()
    {
        changeCheckedState(INTERMEDIATE);
    }


    void CheckBox::resizeCaption()
    {
     
        int x = 0;
        int y = 0;
        int sizeX = 0;
        int sizeY = 0;

        switch (getCheckBoxAlignment())
        {
        case ALIGN_TOP_LEFT:
        case ALIGN_MIDDLE_LEFT:
        case ALIGN_BOTTOM_LEFT:
            x += getCheckBoxSize().x;
            x += getSidePadding();
            break;
        case ALIGN_TOP_CENTER:
            y += getCheckBoxSize().y;
            break;
        case ALIGN_BOTTOM_CENTER:
            sizeY -= getCheckBoxSize().y;
            break;
        case ALIGN_TOP_RIGHT:
        case ALIGN_MIDDLE_RIGHT:
        case ALIGN_BOTTOM_RIGHT:
            sizeX -= getCheckBoxSize().x;
            sizeX += getSidePadding();
            x += getSidePadding();
            break;
        default: break;
        }

            sizeX -= x;
            sizeY -= y;

        Recti areaRect = getInnerRectangle();
        
        sizeX += areaRect.width;
        sizeY += areaRect.height;


        wordWrapRect = Recti(x,y,sizeX,sizeY);
    }

    void CheckBox::setCheckBoxAlignment( AreaAlignmentEnum alignment )
    {
        checkBoxAlignment = alignment;
        resizeCaption();
        positionCheckBox();
        if(isAutosizing())
        {
            resizeToContents();
        }
        textAreaMan.makeTextLines(
            getFont(), getText(), wordWrappedLines, wordWrapRect.width);

        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it))
                (*it)->checkBoxAlignmentChanged(this,alignment);
        }
    }

    AreaAlignmentEnum CheckBox::getCheckBoxAlignment() const
    {
        return checkBoxAlignment;
    }

    void CheckBox::setTextAlignment( AreaAlignmentEnum alignment )
    {
        textAlignment = alignment;
        positionCheckBox();
        if(isAutosizing())
        {
            resizeToContents();
        }
        textAreaMan.makeTextLines(getFont(),getText(),
            wordWrappedLines,wordWrapRect.width);

        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it))
                (*it)->textAlignmentChanged(this,alignment);
        }

    }

    AreaAlignmentEnum CheckBox::getTextAlignment() const
    {
        return textAlignment;
    }

    void CheckBox::positionCheckBox()
    {
        checkBoxPosition = createAlignedPosition(
            getCheckBoxAlignment(),getInnerRectangle(),
            getCheckBoxSize());
        checkBoxRect = Recti(checkBoxPosition,checkBoxSize);
    }


    void CheckBox::paintComponent( const PaintEvent &paintEvent )
    {
        //draw the checkbox
        Recti checkBoxRect = getCheckBoxRectangle();
        Color checkFillColor = Color(255,255,255);
        if(getCheckBoxState() == CLICKED)
        {
            checkFillColor = Color(50,95,128);
        }
        else if(getCheckBoxState() == HOVERED)
        {
            checkFillColor = Color(200,220,230);
        }

        paintEvent.graphics()->drawFilledRectangle(checkBoxRect,checkFillColor);

        //draw the check mark if needed

        checkBoxRect = Recti(
            checkBoxRect.x + (int)(checkBoxRect.width  * 0.25),
            checkBoxRect.y + (int)(checkBoxRect.height * 0.25),
            1 + checkBoxRect.width / 2,1 + checkBoxRect.height / 2);

        Color black = Color(0,0,0);

        Vector2i leftMidPoint = Vector2i(checkBoxRect.x,
            (checkBoxRect.y + checkBoxRect.getBottom()) / 2);

        Vector2i bottomMidPoint = Vector2i(
            (checkBoxRect.x + checkBoxRect.getRight()) / 2,
            checkBoxRect.getBottom());

        switch(getCheckedState())
        {
        case INTERMEDIATE:
            paintEvent.graphics()->drawFilledRectangle(checkBoxRect,black);
            break;
        case CHECKED:
            paintEvent.graphics()->drawLine(checkBoxRect.getTopRight(),
                bottomMidPoint,black);

            paintEvent.graphics()->drawLine(leftMidPoint,bottomMidPoint,black);

            break;
        default:
            break;
        }

        paintEvent.graphics()->drawRectangle(getCheckBoxRectangle(),
            Color(110,110,120,255));

        //draw text
        textAreaMan.drawTextArea(paintEvent.graphics(),getFont(),
            getWordWrapRect(),getFontColor(),
            getTextLines(),getTextAlignment());
    }



    void CheckBox::resizeToContents()
    {
        positionCheckBox();

        if(getText().length() == 0)
        {
            _setSizeInternal(getCheckBoxSize());
            return;
        }
        
        int sizeX = getFont()->getTextWidth(getText());
        int sizeY = getFont()->getLineHeight();


        if(getCheckBoxSize().y > sizeY)
        {
            sizeY = getCheckBoxSize().y;
        }

        
        switch (getCheckBoxAlignment())
        {
        case ALIGN_TOP_CENTER:
        case ALIGN_BOTTOM_CENTER:
        sizeY += getCheckBoxSize().y;
        sizeY += getSidePadding() * 2;
        break;
        case ALIGN_MIDDLE_CENTER:
            break;
        default:
            sizeX += getCheckBoxSize().x;
            sizeX += getSidePadding() * 2;
            break;
        }

        _setSizeInternal(Vector2i(sizeX + 
            getMargin(SIDE_LEFT) + 
            getMargin(SIDE_RIGHT),
            sizeY + 
            getMargin(SIDE_TOP) +
            getMargin(SIDE_BOTTOM))
            );
        resizeCaption();


    }


    bool CheckBox::isAutosizing()
    {
        return autosizingCheckbox;
    }

    void CheckBox::setAutosizing( bool autosizing )
    {
        autosizingCheckbox = autosizing;
        if(isAutosizing())
        {
            resizeToContents();
        }

        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it))
                (*it)->isAutosizingChanged(this,autosizing);
        }
    }

    void CheckBox::setFontColor( const Color &color )
    {
        Widget::setFontColor(color);
    }

    void CheckBox::_setSizeInternal( const Vector2i &size )
    {
        Widget::setSize(size);
        resizeCaption();
        positionCheckBox();
        textAreaMan.makeTextLines(getFont(),getText(),
            wordWrappedLines,wordWrapRect.width);
        
    }


    void CheckBox::setChecked( bool checked )
    {
        if(checked)
        {
            changeCheckedState(CHECKED);
        }
        else
        {
            changeCheckedState(UNCHECKED);
        }
    }

    bool CheckBox::checked() const
    {
        if(getCheckedState() == CHECKED)
        {
            return true;
        }
        return false;
    }

    void CheckBox::addCheckBoxListener( CheckBoxListener* listener )
    {
        if(!listener)
        {
            return;
        }
        for(Vector<CheckBoxListener*>::iterator it = 
            checkBoxListeners.begin();
            it != checkBoxListeners.end(); ++it)
        {
            if((*it) == listener)
                return;
        }

        checkBoxListeners.push_back(listener);
    }

    void CheckBox::removeCheckBoxListener( CheckBoxListener* listener )
    {
        checkBoxListeners.erase(
            std::remove(checkBoxListeners.begin(),
            checkBoxListeners.end(), listener),
            checkBoxListeners.end());
    }


    void CheckBox::mouseEnter( MouseEvent &mouseEvent )
    {
        mouseIsInside = true;
        mouseEvent.consume();
        modifyCheckBoxState();
    }

    void CheckBox::setLocation( const Vector2i &location )
    {
        Widget::setLocation(location);
    }

    void CheckBox::setLocation( int x, int y )
    {
        Widget::setLocation(x,y);
    }

    void CheckBox::setFont( const Font *font )
    {
        Widget::setFont(font);
        if(isAutosizing())
        {
            resizeToContents();
        }
        textAreaMan.makeTextLines(getFont(),getText(),
            wordWrappedLines,wordWrapRect.width);
    }

    void CheckBox::setSize( const Vector2i &size )
    {
        if(!isAutosizing())
        {
            _setSizeInternal(size);
        }
    }

    void CheckBox::setSize( int width, int height )
    {
        Widget::setSize(width,height);
    }

    void CheckBox::setText( const String &text )
    {
        Widget::setText(text);
        resizeCaption();
        if(isAutosizing())
        {
            resizeToContents();
        }
        textAreaMan.makeTextLines(
            getFont(), getText(), wordWrappedLines,wordWrapRect.width);
    }

    void CheckBox::focusGained()
    {
        Widget::focusGained();
        isDoingKeyAction = false;
        modifyCheckBoxState();
    }

    void CheckBox::focusLost()
    {
        Widget::focusLost();
        isDoingKeyAction = false;
        modifyCheckBoxState();
    }

    void CheckBox::mouseLeave( MouseEvent &mouseEvent )
    {
        mouseIsInside = false;
        mouseEvent.consume();
        modifyCheckBoxState();
    }

    void CheckBox::mouseDown( MouseEvent &mouseEvent )
    {
        if(mouseEvent.getButton() == MOUSE_BUTTON_LEFT)
        {
            mouseIsDown = true;
            mouseEvent.consume();
        }

        modifyCheckBoxState();
    }

    void CheckBox::mouseUp( MouseEvent &mouseEvent )
    {
        if(mouseEvent.getButton() == MOUSE_BUTTON_LEFT)
        {
            mouseIsDown = false;
            mouseEvent.consume();
        }

        modifyCheckBoxState();
    }

    void CheckBox::mouseClick( MouseEvent &mouseEvent )
    {
        if(mouseEvent.getButton() == MOUSE_BUTTON_LEFT)
        {
            nextCheckState();
            mouseEvent.consume();
        }
    }

    void CheckBox::keyDown( KeyEvent &keyEvent )
    {
        
        if(keyEvent.getKey() == KEY_SPACE || keyEvent.getKey() == KEY_ENTER)
        {
            isDoingKeyAction = true;
            changeCheckBoxState(CLICKED);
            keyEvent.consume();
        }
    }

    void CheckBox::keyUp( KeyEvent &keyEvent )
    {
        if(!isDoingKeyAction)
        {
            return;
        }
        isDoingKeyAction = false;
        if(keyEvent.getKey() == KEY_SPACE || keyEvent.getKey() == KEY_ENTER)
        {
            dispatchActionEvent(ActionEvent(
                this));
            modifyCheckBoxState();
            nextCheckState();
            keyEvent.consume();
        }
    }

    void CheckBox::paintBackground( const PaintEvent &paintEvent )
    {
    }

    const Vector2i& CheckBox::getCheckBoxPosition() const
    {
        return checkBoxPosition;
    }

    const Recti& CheckBox::getCheckBoxRectangle() const
    {
        return checkBoxRect;
    }

    int CheckBox::getSidePadding() const
    {
        return sidePadding;
    }

    void CheckBox::setSidePadding(int padding) 
    {
        sidePadding = padding;

        positionCheckBox();
        if(isAutosizing())
        {
            resizeToContents();
        }
        textAreaMan.makeTextLines(getFont(),getText(),
            wordWrappedLines,wordWrapRect.width);

    }

    const Vector<String>& CheckBox::getTextLines() const
    {
        return wordWrappedLines;
    }

    const Recti& CheckBox::getWordWrapRect() const
    {
        return wordWrapRect;
    }

}
