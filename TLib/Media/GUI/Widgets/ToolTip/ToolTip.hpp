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

#include <TLib/Media/GUI/Widget.hpp>
namespace agui {

     /**
     * Class that represents a ToolTip that can pop up when hovering over things.
     *
     * ActionEvent when:
     *
     * Clicked.
     * @author Joshua Larouche
     * @since 0.2.0
     */
    class AGUI_CORE_DECLSPEC ToolTip : public Widget {
    private:
        Vector<String> wrappedText;
        AreaAlignmentEnum align;
        Vector2i preferredOffset;
        Widget* invoker;

    protected:
        ResizableText resizableText;
        virtual void paintComponent(const PaintEvent &paintEvent);
        virtual void paintBackground(const PaintEvent &paintEvent);
    public:
        ToolTip();
    /** Returns each line of text. One line per string.
    * @since 0.2.0
    */
        const Vector<String>& getAreaText() const;
    /** Shows the ToolTip at x and y relative to the invoker. Text will wrap to the specified width.
    * A width of 0 can be passed for a single line of with up to 1000px.
    * @see Gui for using a ToolTip as a Gui-Wide object. Every Widget has custom ToolTip text.
    * @since 0.2.0
    */
        virtual void showToolTip(const String& text, int width, int x, int y, Widget* invoker);
            /** Hides the ToolTip
    * @since 0.2.0
    */
        virtual void hideToolTip();
        virtual void mouseClick(MouseEvent &mouseEvent);
        virtual void setTextAlignment(AreaAlignmentEnum alignment);
        virtual AreaAlignmentEnum getTextAlignment() const;
      /** Sets an offset from the top left location of the ToolTip when it is shown.
    * @since 0.2.0
    */
        virtual void setPreferredOffset(Vector2i offset);
             /** @return The offset from the top left location of the ToolTip when it is shown.
    * @since 0.2.0
    */
        virtual Vector2i getPreferredOffset() const;
             /** @return The invoker passed when the ToolTip was shown.
    * @since 0.2.0
    */
        Widget* getInvoker() const;

    };
}
