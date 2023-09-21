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

#include <TLib/Media/GUI/ResizableBorderLayout.hpp>

namespace agui
{
	ResizableBorderLayout::ResizableBorderLayout(void)
		: resizingNorth(true), resizingSouth(true),
        resizingEast(true), resizingWest(true),
        resizing(true), constrainToCenter(true),
        mouseResult(CENTER), oldMargin(0), dragX(0), dragY(0)
	{
	}

	ResizableBorderLayout::~ResizableBorderLayout(void)
	{
	}

	BorderLayout::BorderLayoutEnum ResizableBorderLayout::getPointRegion( const Vector2i &p )
	{
		if(!getWidget(CENTER))
		{
			return CENTER;
		}

		Widget* north = getWidget(NORTH);
		Widget* south = getWidget(SOUTH);
		Widget* east = getWidget(EAST);
		Widget* west = getWidget(WEST);
		Widget* center = getWidget(CENTER);

		if(resizingWest && west)
		{
			if(p.x >= west->getInnerWidth() + west->getLocation().x && p.x <= center->getLocation().x)
			{
				return WEST;
			}
		}

		if(resizingEast && east)
		{
			if(p.x <= east->getLocation().x
				&& p.x >= center->getLocation().x + center->getInnerWidth())
			{
				return EAST;
			}
		}


		if(resizingNorth && north)
		{
			if(p.y >= north->getInnerHeight() + north->getLocation().y
				&& p.y <= center->getLocation().y)
			{
				return NORTH;
			}
		}

		if(resizingSouth && south)
		{
			if(p.y <= south->getLocation().y
				&& p.y >= center->getLocation().y + center->getInnerHeight())
			{
				return SOUTH;
			}
		}

		return CENTER;
		
	}

	void ResizableBorderLayout::mouseDown( MouseEvent &mouseEvent )
	{
		if(mouseEvent.getButton() != MOUSE_BUTTON_LEFT ||
			!isResizable())
		{
			return;
		}

		mouseResult = getPointRegion(mouseEvent.getPosition());
		dragX = mouseEvent.getX();
		dragY = mouseEvent.getY();

		if(mouseResult == NORTH)
		{
			oldMargin = getWidget(NORTH)->getHeight();
		}
		else if(mouseResult == SOUTH)
		{
			oldMargin = getWidget(SOUTH)->getHeight();
		}
		else if(mouseResult == EAST)
		{
			oldMargin = getWidget(EAST)->getWidth();
		}
		else if(mouseResult == WEST)
		{
			oldMargin = getWidget(WEST)->getWidth();
		}
	}

	void ResizableBorderLayout::mouseDrag( MouseEvent &mouseEvent )
	{
		if(mouseResult == CENTER || !isResizable())
		{
			return;
		}

		int delta = mouseResult == NORTH || mouseResult == SOUTH ?
			mouseEvent.getY() - dragY : mouseEvent.getX() - dragX;

		if(mouseResult == SOUTH || mouseResult == EAST)
		{
			delta = -delta;
		}

		int deltaMargin = delta + oldMargin;

		if(constrainToCenter)
		{
			//North
			if( mouseResult == NORTH && deltaMargin > 
				getWidget(CENTER)->getLocation().y - 
				getHorizontalSpacing() + getWidget(CENTER)->getSize().y)
			{
				deltaMargin = getWidget(CENTER)->getLocation().y -
					getHorizontalSpacing() + getWidget(CENTER)->getSize().y;
			}

			//West
			if( mouseResult == WEST && deltaMargin > 
				getWidget(CENTER)->getLocation().x - 
				getVerticalSpacing() + getWidget(CENTER)->getSize().x)
			{
				deltaMargin = getWidget(CENTER)->getLocation().x -
					getVerticalSpacing() + getWidget(CENTER)->getSize().x;
			}

			//South
			if(mouseResult == SOUTH && deltaMargin > getInnerHeight() -
				getWidget(CENTER)->getLocation().y - getHorizontalSpacing())
			{
				deltaMargin = getInnerHeight() -  
					getWidget(CENTER)->getLocation().y - getHorizontalSpacing();
			}

			//East
			if(mouseResult == EAST && deltaMargin > getInnerWidth() - 
				getWidget(CENTER)->getLocation().x - getVerticalSpacing())
			{
				deltaMargin = getInnerWidth() -  
					getWidget(CENTER)->getLocation().x - getVerticalSpacing();
			}
		}
		
		setBorderMargin(mouseResult,deltaMargin);
	}

	void ResizableBorderLayout::setConstrainToCenter(bool constrain)
	{
		constrainToCenter = constrain;
		updateLayout();
	}

	bool ResizableBorderLayout::isConstrainedToCenter() const
	{
		return constrainToCenter;
	}

	void ResizableBorderLayout::setResizable( bool resize )
	{
		resizing = resize;
	}

	bool ResizableBorderLayout::isResizable() const
	{
		return resizing;
	}

}
