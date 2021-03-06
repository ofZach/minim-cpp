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

#include "UGen.h"
#include "AudioSystem.h"
#include "AudioOutput.h"

namespace Minim
{

///////////////////////////////////////////////////////////////
UGen::UGenInput::UGenInput( UGen & outerUGen, UGen::InputType inputType )
: mOuterUGen(outerUGen) 
, mInputType(inputType)
, mIncoming(NULL)
{
	mOuterUGen.mUGenInputs.push_back(this);
}

///////////////////////////////////////////////////
const char * UGen::UGenInput::getInputTypeAsString() const
{
	switch (mInputType)
	{
	case AUDIO :
		return "AUDIO";
		break;
	case CONTROL :
		return "CONTROL";
		break;
	}
	return "";
}

///////////////////////////////////////////////////
void UGen::UGenInput::printInput() const
{
	char msg[256];
	sprintf( msg, "UGenInput: signal = %s %i", getInputTypeAsString(), isPatched() );
	Minim::debug( msg );
}

///////////////////////////////////////////////////
UGen::UGen()
: mNumOutputs(0)
, mCurrentTick(0)
, mLastValues(0)
, mLastValuesLength(0)
{

}

///////////////////////////////////////////////////
UGen & UGen::patch( UGen & connectToUGen )
{
	connectToUGen.addInput( this );

	// TODO jam3: nOutputs should only increase when this chain will be ticked!
	mNumOutputs += 1;

	return connectToUGen;
}

////////////////////////////////////////////////////
UGen & UGen::patch( UGenInput & connectToInput )
{
	connectToInput.setIncomingUGen( this );

	mNumOutputs += 1;

	return connectToInput.getOuterUGen();
}

///////////////////////////////////////////////////
void UGen::patch( AudioOutput & output )
{
	patch( output.mSummer );
	setSampleRate( output.sampleRate() );
}

///////////////////////////////////////////////////
void UGen::unpatch( AudioOutput & output )
{
	unpatch( output.mSummer );
}

///////////////////////////////////////////////////
void UGen::unpatch( UGen & disconnectFrom )
{
	disconnectFrom.removeInput( this );
	mNumOutputs -= 1;
}

//////////////////////////////////////////////////
void UGen::addInput( UGen * input )
{
	// jam3: This default behavior is that the incoming signal will be added
	// 		to the first input in the uGenInputs list.
	Minim::debug("UGen addInput called.");
	// TODO change input checking to an Exception?
	if ( mUGenInputs.size() > 0 )
	{
		Minim::debug("Initializing default input on something");	
		UGenInput* pFirstInput = *mUGenInputs.begin();
		pFirstInput->setIncomingUGen( input );
	}  
	else
	{
		Minim::error("Trying to connect to UGen with no default input.");
	}
}

///////////////////////////////////////////////////
void UGen::removeInput(Minim::UGen *input)
{
	Minim::debug("UGen removeInput called.");
	// TODO Need to set default behavior for normal UGens on removeInput
}


///////////////////////////////////////////////////
void UGen::tick(float *channels, int numChannels)
{
	if (mNumOutputs > 0)
	{
		// only tick once per sampleframe when multiple outputs
		mCurrentTick = (mCurrentTick + 1)%(mNumOutputs);
	}

	if (0 == mCurrentTick) 
	{			
		if ( mUGenInputs.size() > 0 )
		{
			std::list<UGenInput*>::iterator itr = mUGenInputs.begin();
			for(; itr != mUGenInputs.end(); ++itr)
			{		
				UGenInput* pInput = *itr;
				if ( pInput->isPatched() )
				{
					float * tmp = NULL;
					int     tmpSize = 1;
					switch ( pInput->getInputType() )
					{
					case CONTROL :
						tmp = new float[1];
						break;
					default : // includes AUDIO
						tmp = new float[ numChannels ];
						tmpSize = numChannels;
						break;
					}
					// tick all connected inputs
					pInput->getIncomingUGen().tick( tmp, tmpSize );
				}
			}
		}
		// and then uGenerate for this UGen	
		uGenerate( channels, numChannels );

		// make sure our last values array matches with the size of channels
		if ( mLastValuesLength != numChannels )
		{
			if ( mLastValues )
			{
				delete [] mLastValues;
			}
			
			mLastValues = new float[numChannels];
		}
		// need to keep the last values generated so we have something to hand multiple outputs 
		memcpy(mLastValues, channels, sizeof(float) * numChannels);
	}
}

/////////////////////////////////////////////////////
void UGen::setSampleRate(float newSampleRate)
{
	if ( mSampleRate != newSampleRate)
	{
		mSampleRate = newSampleRate;
		sampleRateChanged();
	}
	if ( mUGenInputs.size() > 0 )
	{
		std::list<UGenInput*>::iterator itr = mUGenInputs.begin();
		for(; itr != mUGenInputs.end(); ++itr)
		{		
			UGenInput* pInput = *itr;
			if ( pInput->isPatched() )
			{
				pInput->getIncomingUGen().setSampleRate(newSampleRate);
			}
		}			
	}
}

////////////////////////////////////////////////////
void UGen::printInputs() const
{
   std::list<UGenInput*>::const_iterator itr = mUGenInputs.begin();
   for(int i = 0; itr != mUGenInputs.end(); ++itr, ++i)
   {
	   char msg[64];
	   sprintf( msg, "uGenInputs %d ", i );
	   Minim::debug(msg);
	   (*itr)->printInput();
   }
}

} // namespace Minim
