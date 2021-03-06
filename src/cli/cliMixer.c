/*

FF32lite from FocusFlight, a new alternative firmware
for the Naze32 controller

Original work Copyright (c) 2013 John Ihlein

This file is part of FF32lite.

Includes code and/or ideas from:

  1)BaseFlight
  2)S.O.H. Madgwick

FF32lite is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FF32lite is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FF32lite. If not, see <http://www.gnu.org/licenses/>.

*/

///////////////////////////////////////////////////////////////////////////////

#include "board.h"

///////////////////////////////////////////////////////////////////////////////
// Mixer CLI
///////////////////////////////////////////////////////////////////////////////

void mixerCLI()
{
    float    tempFloat;

    uint8_t  index;
    uint8_t  rows, columns;

    uint8_t  mixerQuery = 'x';
    uint8_t  validQuery = false;

    cliBusy = true;

    cliPortPrint("\nEntering Mixer CLI....\n\n");

    while(true)
    {
        cliPortPrint("Mixer CLI -> ");

		while ((cliPortAvailable() == false) && (validQuery == false));

		if (validQuery == false)
		    mixerQuery = cliPortRead();

		cliPortPrint("\n");

		switch(mixerQuery)
		{
            ///////////////////////////

            case 'a': // Mixer Configuration
                cliPortPrint("\nMixer Configuration:  ");
                switch (eepromConfig.mixerConfiguration)
                {
                    case MIXERTYPE_TRI:
                        cliPortPrint("   MIXERTYPE TRI\n");
                        break;

                    case MIXERTYPE_QUADX:
                        cliPortPrint("MIXERTYPE QUAD X\n");
                        break;

                    case MIXERTYPE_HEX6X:
                        cliPortPrint(" MIXERTYPE HEX X\n");
                        break;

                    case MIXERTYPE_FREE:
                        cliPortPrint("  MIXERTYPE FREE\n");
                        break;
                }

                cliPortPrintF("Number of Motors:                    %1d\n",  numberMotor);
                cliPortPrintF("ESC PWM Rate:                      %3ld\n",   eepromConfig.escPwmRate);
                cliPortPrintF("Servo PWM Rate:                    %3ld\n\n", eepromConfig.servoPwmRate);

                if (eepromConfig.yawDirection == 1.0f)
                	cliPortPrintF("Yaw Direction:                  Normal\n\n");
                else if (eepromConfig.yawDirection == -1.0f)
                	cliPortPrintF("Yaw Direction:                 Reverse\n\n");
                else
                	cliPortPrintF("Yaw Direction:               Undefined\n\n");

                if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
                {
					cliPortPrintF("TriCopter Yaw Servo PWM Rate:      %3ld\n",  eepromConfig.triYawServoPwmRate);
                    cliPortPrintF("TriCopter Yaw Servo Min PWM:      %4ld\n",   (uint16_t)eepromConfig.triYawServoMin);
                    cliPortPrintF("TriCopter Yaw Servo Mid PWM:      %4ld\n",   (uint16_t)eepromConfig.triYawServoMid);
                    cliPortPrintF("TriCopter Yaw Servo Max PWM:      %4ld\n\n", (uint16_t)eepromConfig.triYawServoMax);
                    cliPortPrintF("Tricopter Yaw Cmd Time Constant:  %5.3f\n\n", eepromConfig.triCopterYawCmd500HzLowPassTau);
			    }

        	    if (eepromConfig.mixerConfiguration == MIXERTYPE_FREE)
                {
					cliPortPrintF("\nNumber of Free Mixer Motors:  %1d\n         Roll    Pitch   Yaw\n\n", eepromConfig.freeMixMotors);

        	        for ( index = 0; index < eepromConfig.freeMixMotors; index++ )
        	        {
        	    	    cliPortPrintF("Motor%1d  %6.3f  %6.3f  %6.3f\n", index,
        	    			                                             eepromConfig.freeMix[index][ROLL ],
        	    			                                             eepromConfig.freeMix[index][PITCH],
        	    			                                             eepromConfig.freeMix[index][YAW  ]);
        	        }

        	        cliPortPrint("\n");
			    }

                cliPortPrintF("Roll Att Alt Compensation Limit:  %4.1f\n",   eepromConfig.rollAttAltCompensationLimit * R2D);
                cliPortPrintF("Roll Att Alt Compensation Gain:   %4.1f\n\n", eepromConfig.rollAttAltCompensationGain);

                cliPortPrintF("Pitch Att Alt Compensation Limit: %4.1f\n",   eepromConfig.pitchAttAltCompensationLimit * R2D);
                cliPortPrintF("Pitch Att Alt Compensation Gain:  %4.1f\n\n", eepromConfig.pitchAttAltCompensationGain);

                validQuery = false;
                break;

            ///////////////////////////

			case 'x':
			    cliPortPrint("\nExiting Mixer CLI....\n\n");
			    cliBusy = false;
			    return;
			    break;

            ///////////////////////////

            case 'A': // Read Mixer Configuration
                eepromConfig.mixerConfiguration = (uint8_t)readFloatCLI();

                initMixer();
                pwmEscInit();

        	    mixerQuery = 'a';
                validQuery = true;
		        break;

            ///////////////////////////

            case 'B': // Read ESC and Servo PWM Update Rates
                eepromConfig.escPwmRate   = (uint16_t)readFloatCLI();
                eepromConfig.servoPwmRate = (uint16_t)readFloatCLI();

                pwmEscInit();
                // HJI pwmServoInit();

                mixerQuery = 'a';
                validQuery = true;
        	    break;

            ///////////////////////////

            case 'D': // Read yaw direction
                tempFloat = readFloatCLI();
                if (tempFloat >= 0.0)
                    tempFloat = 1.0;
                else
                	tempFloat = -1.0;

                eepromConfig.yawDirection = tempFloat;

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'E': // Read TriCopter Servo PWM Rate
            	if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
               	{
               		eepromConfig.triYawServoPwmRate = (uint16_t)readFloatCLI();

                    pwmEscInit();
               	}
                else
                {
                   	tempFloat = readFloatCLI();

                   	cliPortPrintF("\nTriCopter Mixing not Selected....\n\n");
                }

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'F': // Read TriCopter Servo Min Point
               	if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
               	{
               		eepromConfig.triYawServoMin = readFloatCLI();

               		pwmEscWrite(5, (uint16_t)eepromConfig.triYawServoMin);
                }
                else
                {
                   	tempFloat = readFloatCLI();

                    cliPortPrintF("\nTriCopter Mixing not Selected....\n\n");
                }

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'G': // Read TriCopter Servo Mid Point
                if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
                {
                    eepromConfig.triYawServoMid = readFloatCLI();

                    pwmEscWrite(5, (uint16_t)eepromConfig.triYawServoMid);
                }
                else
                {
                   	tempFloat = readFloatCLI();

                   	cliPortPrintF("\nTriCopter Mixing not Selected....\n\n");
                }

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'H': // Read TriCopter Servo Max Point
                if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
                {
                    eepromConfig.triYawServoMax = readFloatCLI();

                    pwmEscWrite(5, (uint16_t)eepromConfig.triYawServoMax);
                }
                else
                {
                    tempFloat = readFloatCLI();

                    cliPortPrintF("\nTriCopter Mixing not Selected....\n\n");
                }

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'I': // Read TriCopter Yaw Command Time Constant
                if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
                {
                	eepromConfig.triCopterYawCmd500HzLowPassTau = readFloatCLI();

                	initFirstOrderFilter();
                }
                else
                {
                    tempFloat = readFloatCLI();

                    cliPortPrintF("\nTriCopter Mixing not Selected....\n\n");
                }

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'J': // Read Free Mix Motor Number
           	    if (eepromConfig.mixerConfiguration == MIXERTYPE_FREE)
                {
                	eepromConfig.freeMixMotors = (uint8_t)readFloatCLI();
           	        initMixer();
				}
				else
				{
					tempFloat = readFloatCLI();

					cliPortPrintF("\nFree Mix not Selected....\n\n");
                }

           	    mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'K': // Read Free Mix Matrix Element
                if (eepromConfig.mixerConfiguration == MIXERTYPE_FREE)
                {
                	rows    = (uint8_t)readFloatCLI() - 1;
                    columns = (uint8_t)readFloatCLI() - 1;

                    eepromConfig.freeMix[rows][columns] = readFloatCLI();
				}
				else
				{
					tempFloat = readFloatCLI();
					tempFloat = readFloatCLI();
					tempFloat = readFloatCLI();

					cliPortPrintF("\nFree Mix not Selected....\n\n");
                }

           	    mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'U': // Roll Att Alt Compensation Limit and Gain
            	eepromConfig.rollAttAltCompensationLimit = readFloatCLI() * D2R;
                eepromConfig.rollAttAltCompensationGain  = readFloatCLI();

               	mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'V': // PitchAtt Alt Compensation Limit and Gain
            	eepromConfig.pitchAttAltCompensationLimit = readFloatCLI() * D2R;
            	eepromConfig.pitchAttAltCompensationGain  = readFloatCLI();

                mixerQuery = 'a';
                validQuery = true;
                break;

            ///////////////////////////

            case 'W': // Write EEPROM Parameters
                cliPortPrint("\nWriting EEPROM Parameters....\n\n");
                writeEEPROM();

                validQuery = false;
                break;

			///////////////////////////

			case '?':
			   	cliPortPrint("\n");
			   	cliPortPrint("'a' Mixer Configuration Data               'A' Set Mixer Configuration              A0 thru 3, see ff32_Naze32.h\n");
   		        cliPortPrint("                                           'B' Set PWM Rates                        BESC;Servo\n");
   		        cliPortPrint("                                           'D' Set Yaw Direction                    D1 or D-1\n");

   		        if (eepromConfig.mixerConfiguration == MIXERTYPE_TRI)
   		    	{
   		        	cliPortPrint("                                           'E' Set TriCopter Servo PWM Rate         ERate\n");
   		        	cliPortPrint("                                           'F' Set TriCopter Servo Min Point        FMin\n");
			   	    cliPortPrint("                                           'G' Set TriCopter Servo Mid Point        GMid\n");
			   	    cliPortPrint("                                           'H' Set TriCopter Servo Max Point        HMax\n");
			   	    cliPortPrint("                                           'I' Set TriCopter Yaw Cmd Time Constant  ITimeConstant\n");
   		    	}

   		        if (eepromConfig.mixerConfiguration == MIXERTYPE_FREE)
   		    	{
   		        	cliPortPrint("                                           'J' Set Number of FreeMix Motors         JNumb\n");
   		        	cliPortPrint("                                           'K' Set FreeMix Matrix Element           KRow;Col;Value\n");
			   	}

   		        cliPortPrint("                                           'U' Roll Att Alt Comp Limit;Gain         UrLimit;rGain\n");
   		        cliPortPrint("                                           'V' Pitch Att Alt Comp Limit;Gain        VpLimit;pGain\n");
   		        cliPortPrint("                                           'W' Write EEPROM Parameters\n");
   		        cliPortPrint("'x' Exit Mixer CLI                         '?' Command Summary\n");
   		        cliPortPrint("\n");
	    	    break;

	    	///////////////////////////
	    }
	}
}

///////////////////////////////////////////////////////////////////////////////
