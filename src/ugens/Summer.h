/*
 *   Author: Damien Di Fede <ddf@compartmental.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as published
 *   by the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef SUMMER_H
#define SUMMER_H

#include "UGen.h"
#include <vector>

namespace Minim
{
	class AudioOutput;

	class Summer : public UGen
	{

	public:
		Summer();
	

	protected:
		// ddf: I do not expect users of the library to construct busses that they pass their output to.
		//      This exists so that we can tick the noteManager for the provided output.
		//      In other words, the output passed to this constructor will always be the same output
		//      that is constructing the bus.
		friend AudioOutput;
		Summer(AudioOutput * output);
	
	// ddf: override because everything that patches to us
	//      goes into our list. then when we generate a sample
	//      we'll sum the audio generated by all of the ugens patched to us.
	// Override
	virtual void addInput(UGen * input)
	{
		// Minim.debug("Bus::addInput - Adding " + input + " to the ugens list of " + this);
		mUGens.push_back(input);
	}
	
	// Override
	virtual void removeInput(UGen * input)
	{
		// Minim.debug("Bus::removeInput - Removing " + input + " to the ugens list of " + this);
		mToRemove.push_back(input);
	}

	// override
	virtual void sampleRateChanged();

	// UGen impl
	virtual void uGenerate(float * channels, int numChannels);

	private:
		AudioOutput * mOutput;
		std::vector<UGen*> mUGens;
		std::vector<UGen*> mToRemove;
	};
};

#endif // SUMMER_H