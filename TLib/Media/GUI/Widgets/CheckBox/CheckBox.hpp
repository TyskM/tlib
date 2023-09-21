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

#ifndef AGUI_CHECKBOX_HPP
#define AGUI_CHECKBOX_HPP

#include <TLib/Media/GUI/Widget.hpp>
namespace agui {
	class AGUI_CORE_DECLSPEC CheckBoxListener;
	/**
	 * Class that represents a CheckBox that can be CHECKED, UNCHECKED, or INTERMEDIATE.
	 *
	 * ActionEvent when:
	 *
	 * Checked changes.
	 *
     * @author Joshua Larouche
     * @since 0.1.0
     */
	class AGUI_CORE_DECLSPEC CheckBox : public Widget
	{
	public:
		enum CheckBoxStateEnum {
			DEFAULT,
			HOVERED,
			CLICKED
		};

		enum CheckBoxCheckedEnum {
			UNCHECKED,
			CHECKED,
			INTERMEDIATE
		};

	private:
		int sidePadding;
		Recti wordWrapRect;
		Vector<String> wordWrappedLines;
		Vector<CheckBoxListener*> checkBoxListeners;
		bool autosizingCheckbox;
		Vector2i checkBoxSize;
		Vector2i checkBoxPosition;
		Recti checkBoxRect;
		AreaAlignmentEnum textAlignment;
		AreaAlignmentEnum checkBoxAlignment;
		CheckBox::CheckBoxStateEnum checkBoxState;
		CheckBox::CheckBoxCheckedEnum checkedState;
		bool mouseIsInside;
		bool mouseIsDown;
		bool isDoingKeyAction;
	protected:
		ResizableText textAreaMan;
	 /**
	 * @return Vector of strings used to draw the text.
     * @since 0.1.0
     */
		virtual const Vector<String>& getTextLines() const;

	 /**
	 * @return Recti provided when drawing the text.
     * @since 0.1.0
     */
		virtual const Recti& getWordWrapRect() const;
	 /**
	 * Internally sets the widget size.
     * @since 0.1.0
     */
		void _setSizeInternal(const Vector2i &size);
	 /**
	 * Internally changes the CheckBox's state.
     * @since 0.1.0
     */
		void changeCheckBoxState(CheckBox::CheckBoxStateEnum state);
	/**
	 * Internally changes the checked state.
     * @since 0.1.0
     */
		void changeCheckedState(CheckBox::CheckBoxCheckedEnum state);
	/**
	 * Internally modifies the CheckBox state.
     * @since 0.1.0
     */
		void modifyCheckBoxState();
	/**
	 * Internally changes to the next logical checked state.
     * @since 0.1.0
     */
		virtual void nextCheckState();
	/**
	 * Generates the Recti used to draw the CheckBox.
     * @since 0.1.0
     */
		virtual void positionCheckBox();
			/**
	 * @return The position of the actual CheckBox.
     * @since 0.1.0
     */
		virtual const Vector2i& getCheckBoxPosition() const;
		/**
	 * Internally resizes the caption text.
     * @since 0.1.0
     */
		virtual void resizeCaption();
	/**
	 * @return The Recti used to draw the CheckBox itself.
     * @since 0.1.0
     */
		virtual const Recti& getCheckBoxRectangle() const;

		virtual void paintComponent(const PaintEvent &paintEvent);
		virtual void paintBackground(const PaintEvent &paintEvent);
	public:
	/**
	 * @return The side padding. This pads symmetrically from left to right or 
	 * top to bottom depending on alignment.
     * @since 0.1.0
     */
		virtual int getSidePadding() const;
	/**
	 * Sets the side padding. This pads symmetrically from left to right or 
	 * top to bottom depending on alignment.
     * @since 0.1.0
     */
		virtual void setSidePadding(int padding);
		virtual void setLocation(const Vector2i &location);
		virtual void setLocation(int x, int y);
		virtual void setFont(const Font *font);
		virtual void setSize(const Vector2i &size);
		virtual void setSize(int width, int height);
		virtual void setText(const String &text);
		virtual void focusGained();
		virtual void focusLost();
		virtual void mouseEnter(MouseEvent &mouseEvent);
		virtual void mouseLeave(MouseEvent &mouseEvent);
		virtual void mouseDown(MouseEvent &mouseEvent);
		virtual void mouseUp(MouseEvent &mouseEvent);
		virtual void mouseClick(MouseEvent &mouseEvent);
		virtual void keyDown(KeyEvent &keyEvent);
		virtual void keyUp(KeyEvent &keyEvent);


	/**
	 * Sets whether or not the CheckBox is checked.
     * @since 0.1.0
     */
		virtual void setChecked(bool checked);
	/**
	 * @return True if the CheckBox is checked.
     * @since 0.1.0
     */
		virtual bool checked() const;
	/**
	 * @return True if the CheckBox is automatically sizing itself.
	 *
	 * If this is true, any calls to setSize will not do anything.
     * @since 0.1.0
     */
		virtual bool isAutosizing();
			/**
	 * Sets if the CheckBox is automatically sizing itself.
	 *
	 * If this is true, any calls to setSize will not do anything.
     * @since 0.1.0
     */
		virtual void setAutosizing(bool autosizing);
	/**
	 * Sets the size of the actual CheckBox.
     * @since 0.1.0
     */
		virtual void setCheckBoxSize(const Vector2i &size);
	/**
	 * @return The size of the actual CheckBox.
     * @since 0.1.0
     */
		virtual const Vector2i& getCheckBoxSize() const;
		virtual void setFontColor(const Color &color);
	/**
	 * Sets the alignment of the actual CheckBox.
     * @since 0.1.0
     */
	virtual void setCheckBoxAlignment(AreaAlignmentEnum alignment);
	/**
	 * @return The alignment of the actual CheckBox.
     * @since 0.1.0
     */
		virtual AreaAlignmentEnum getCheckBoxAlignment() const;
	/**
	 * Sets the alignment of the caption text.
     * @since 0.1.0
     */
		virtual void setTextAlignment(AreaAlignmentEnum alignment);
	/**
	 * @return The alignment of the caption text.
     * @since 0.1.0
     */
		virtual AreaAlignmentEnum getTextAlignment() const;
	/**
	 * Sets the Checked state to INTERMEDIATE.
     * @since 0.1.0
     */
		void setIntermediateState();
	/**
	 * Resizes the CheckBox to a size that fits the caption nicely.
     * @since 0.1.0
     */
		virtual void resizeToContents();

	/**
	 * @return The state of the CheckBox (DEFAULT, HOVERED, CLICKED).
     * @since 0.1.0
     */
		CheckBox::CheckBoxStateEnum getCheckBoxState() const;
	/**
	 * @return The Checked state of the CheckBox (UNCHECKED, CHECKED, INTERMEDIATE).
     * @since 0.1.0
     */
		CheckBox::CheckBoxCheckedEnum getCheckedState() const;

	/**
	 * Default constructor.
     * @since 0.1.0
     */
		CheckBox();

	/**
	 * Adds the parameter CheckBoxListener.
     * @since 0.1.0
     */
		virtual void addCheckBoxListener(
			CheckBoxListener* listener);
	/**
	 * Removes the parameter CheckBoxListener.
     * @since 0.1.0
     */
		virtual void removeCheckBoxListener(
			CheckBoxListener* listener);

	/**
	 * Default destructor.
     * @since 0.1.0
     */
		virtual ~CheckBox(void);
	};
}
#endif
