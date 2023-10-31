# Updating DLD firmware on F446 units

DLD units with serial number 3240 and higher have a different chip than earlier units, and require a different firmware file. These units began shipping to dealers in January 2023, so if you purchased your unit prior to that, then you do not have one of these new chips. 

The new chips are the STM32F446 (or F446 for short) and can be identified by looking at the back of your unit in the top right corner and reading the PCB version number. If the white letters say "v1.2-f446" then you have an F446 unit. If they say anything else then you have a F427 unit.

The document describes how to use the F446 bootloader, since it has subtle differences than the F427 bootloader (which is described in the DLD User Manual).

As of now (Nov 2023), there are no firmware updates available for the DLD, so if you have an F446 unit then you definitely have the latest firmware and do not need to upgrade. Therefore, this document is only relevent to someone wanting to compile their own custom alt firmware.

This section of text should replace the **Audio Bootloader** section in the User Manual (page 12):
 


### Audio Bootloader

The DLD contains a bootloader that is used to update the firmware by playing an audio file the Return A jack on the lefT side of the module. Firmware audio files can be downloaded at http://4mscompany.com/dld.php

1. To enter bootloader mode, power off the DLD and connect a computer or smart phone audio output to the Return A jack. Either a stereo or mono cable is fine. Connect the Send A jack to an amp/speakers so you can listen.
Remove your phone case, it may be preventing the cable from fully plugging in.
       
2. Set the computer/phone's volume to 100% and the audio player software to 100% volume. Turn off all audio and vibrate notifications (use Airplane mode). Close any applications that make notification sounds such as Facebook.
       
3. Depress both Reverse buttons (A and B) and the Ping button while powering on the DLD. When you see the Ping button blink, the DLD is ready to receive firmware. Release the buttons.
       
4. Begin playing the file. Immediately you should see red Channel A Loop LED blink. The blue light will flash as well. Do not interrupt the process! You can monitor the audio by listening to the Send A jack. 
       
5. If the monitored audio stops before the end of the file and/or the lights stop blinking, an error has occurred and you should try again. Verify the cable is not loose, all sounds/vibrate/notifications are off, and that you have downloaded the audio file completely (avoid streaming or playing from the browser). Check the volume is at 100%. Remove the protection case from your smart phone. Stop the audio file, reset it back to the start, and tap Reverse A button to reset. The Ping button should start blinking. Play the file from the beginning again.
       
6. If the file loads successfully, the DLD will immediately start running. We recommend you do a Factory Reset after upgrading to a new version (see Factory Reset section).


### Summary of differences between F446 and F427 bootloaders:

- F446: Return A jack is used for audio input (vs. F427 uses In B)
- F446: Send A is used to monitor the audio (vs. F427 uses Out B)
- F446: When bootloader mode starts, the Ping button blinks (vs. F427 blinks the Channel A Hold button is on)
- The blue light flashes more frequently on the F446 than on the F427 bootloader
