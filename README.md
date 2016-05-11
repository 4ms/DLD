# DLD
## Dual Looping Delay

Firmware for the Dual Looping Delay, a Eurorack-format module from 4ms Company.

The DLD is...

The DLD would make a nice platform for other audio projects, the hardware contains:

*	180MHz 32-bit ARM chip with FPU (STM32F427)
*	Two Stereo codec chip, very low noise and low jitter (CS4721)
	*	Total of four audio inputs and four audio outputs (quadrophonic, anyone...?)
*	Eight potentiometers
*	Six Analogue inputs:
	*	Four unipolar analogue inputs (0 to 5V)
	*	Two bipolar analogue inputs (-5V to 5V)
*	Five LED buttons (momentary)
*	Two three-position switches
*	Five trigger/gate digital inputs
*	Three digital outputs
*	Two LEDs, capable of being dimmed with PWM
*	Separate analogue power supplies for each codec, and also the ADC 
*	EFficient 3.3V DC-DC converter accepts a wide range of input voltages
 

## Setting up your environment
You need to install the GCC ARM toolchain.
This project is known to compile with arm-none-eabi-gcc version 4.8.3, and version 4.9.3. It is likely that it will compile with future versions as well.

It's recommended (but not necessary) to install ST-UTIL/stlink. Without it, you will have to update using the audio bootloader, which is very slow (5 minutes per update).
With ST-UTIL or stlink and a programmer, you can update in 5-20 seconds.
The Texane stlink package contains a gdb debugger, which works with ST-LINK programmers such as the [STM32 Discovery boards](http://www.mouser.com/ProductDetail/STMicroelectronics/STM32F4DISCOVERY/?qs=J2qbEwLrpCGdWLY96ibNeQ%3D%3D&gclid=CKb6u6Cz48cCFZGBfgodfHwH-g&kpid=608656256) to connect to the Spectral's 4-pin SWD header. The STM32F4 Discovery Board is low-cost (under US$15) and works great as a programmer and debugger.

You also may wish to install and IDE such as Eclipse. There are many resources online for setting up GCC ARM with Eclipse (as well as commerical software). This is totally optional. Instead of an IDE you can use your favorite text editor and a few commands on the command line (Terminal) which are given below.

Continue below for Max OSX, Linux, or Windows instructions.


### Mac OSX

For Mac OSX, follow these instructions to install brew and then the arm toolchain and st-link (taken from https://github.com/nitsky/homebrew-stm32):

	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	brew tap nitsky/stm32
	brew install arm-none-eabi-gcc
	brew install stlink

That's it! Continue below to **Clone the Projects**

### Linux

For linux, check your package manager to see if there is a package for arm-none-eabi-gcc or gcc-arm-embedded or gcc-arm-none-eabi. Or just download it here:

[Download GCC ARM toolchain from Launchpad](https://launchpad.net/gcc-arm-embedded/+download)

Next, install st-link from texane:

	sudo apt-get install git libusb-1.0.0-dev pkg-config autotools-dev
	cd (your work directory)
	git clone https://github.com/texane/stlink.git
	cd stlink
	./autogen.sh
	./configure
	make
	export PATH=$PATH:(your work directory)/stlink

The last command makes sure that the binary st-flash is in your PATH so that the Makefile can run it to program your module. 
 
That's it! Continue below to **Clone the Projects**

### Windows
[Download GCC ARM toolchain from Launchpad](https://launchpad.net/gcc-arm-embedded/+download)

[Download ST-UTIL](http://www.st.com/web/en/catalog/tools/PF258168)

Install both. Please contact me if you run into problems that you can't google your way out of! 


## Clone the Projects

Make sure git is installed on your system. (OSX: type "brew install git" into the Terminal)

Create a work directory, and enter it.

Clone this project (DLD), stmlib, and the stm-audio-bootloader projects:

	git clone https://github.com/4ms/DLD.git  
	git clone https://github.com/4ms/stmlib.git
	git clone https://github.com/4ms/stm-audio-bootloader.git
	
Create a symlink for stm-audio-bootloader so that it works with python (required to generate the .wav file for audio bootloading)

     ln -s stm-audio-bootloader stm_audio_bootloader

Verify that your directories are as follows:

	(work directory)
	|  
	|--DLD/  
	|--stm-audio-bootloader/  
	|--stm_audio_bootloader/   <----symlink to stm-audio-bootloader
	|--stmlib/



## Compiling
Make your changes to the code in the DLD directory. When ready to compile, make the project like this:

	cd DLD
	make

If you want to flash the file onto the DLD, attach an [ST-LINK programmer](http://www.st.com/web/catalog/tools/FM116/SC959/SS1532/PF252419) in SWD mode using a 4-conductor cable and run:

	make flash

Read the Discovery Board's manual for how to set it to program external devices. On the STM32F4 Discovery board, you have to remove two DISCOVERY jumpers to put it in ST-LINK mode. Connect the 4-pin cable to the SWD header starting at the dot (pin 1) on the Disco board. On the DLD, pin 1 is labeled with a "1". Any 4-pin cable or even 4 breadboard jumper wires will work. A 10-to-16 pin eurorack power cable is fine too, if it's short enough.

When ready to build an audio file for the bootloader, make it like this:

	make wav

This requires python to run. It creates the file main.wav in the build/ directory. Play the file from a computer or device into the DLD by following the instructions in the User Manual on the [4ms DLD page](http://4mscompany.com/dld.php). 

Troubleshooting: If you have trouble getting python to create a wav file, such as this error:

	ImportError: No module named stm_audio_bootloader

Then try this command:
	
	export PYTHONPATH=$PYTHONPATH:'.'

## Bootloader
The bootloader is a [separate project](https://github.com/4ms/stm-audio-bootloader), slightly modifed from the stm-audio-bootloader from [pichenettes](https://github.com/pichenettes/eurorack). 

The bootloader is already installed on all factory-built DLDs.

## License
The code (software) is licensed by the MIT license.

The hardware is licensed by the [CC BY-NC-SA license](https://creativecommons.org/licenses/by-nc-sa/4.0/) (Creative Commons, Attribution, NonCommercial, ShareAlike).

See LICENSE file.

I would like to see others build and modify the DLD and DLD-influenced works, in a non-commercial manner. My intent is not to limit the educational use nor to prevent people buying hardware components/PCBs collectively in a group. If you have any questions regarding the license or appropriate use, please do not hesitate to contact me! 

## Guidelines for derivative works

Do not include the text "4ms" or "4ms Company" or the graphic 4ms logo on any derivative works. This includes faceplates, enclosures, or front-panels. It's OK (but not required) to include the text "Spectral Multiband Resonator" or "SMR" if you wish.
