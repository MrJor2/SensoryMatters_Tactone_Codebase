# Guide For Handling Tactone CodeBase

Precaution: This codebase is not meant to be educational. Some parts of the code are not relevant to change if you want to use the tactone tool
This tool uses a teensy 4.1 to power all the logic, and a 12V 1A external powersupply, and will need a 5v usb powersupply currently for the microcontroller itself.

## How to use tactone (teensy 4.1) in relation to Arduino IDE
To upload new code to Tactone, you will need the arduino IDE installed, can be found at: https://www.arduino.cc/en/software
You will then need to add the teensy board manager. Which you can do in the arduino IDE, by file>preferences>Additional board managers URLS:
And add the following link: https://www.pjrc.com/teensy/package_teensy_index.json, or if this does not work anymore, check the teensy documentation page at:
https://www.pjrc.com/teensy/td_download.html

Then in the board manager window, download the teensy board manager. See icon in attached image.
![image](https://github.com/user-attachments/assets/0ac9dfa6-e6a6-4775-954b-4aef97a4c76f)

Note: in older versions of arduino, the board manager can be found under tools>board manager

After completing these steps, copy paste the code from this repo in an empty file, and save it, under whatever name you please. 
Compile the code, and see if you are succesfull, otherwise feel free to open an issue on this repo. 

## Changeable Variables
It is relevant to know that there are a few variables that are open to change and finetune when using tactone in a different setting. 

`Lowerbound` and `LDRtriggermap` are variables that can be changed dependend on the light level of the environment.
If an environment is very bright, the trigger value can be very high, and in a darker environment a higher trigger value can be beneficial
but this is subject to finetuning, and where the lightsources are positioned relative to tactone. 
We reccomend having a direct lightsource above tactone for stability. 

`filelist` is a list of audiofiles that are loaded on the SD card. This list consists of 8 files.
Two files for each of the pads, a base pattern (Rythm), which currently remains unmodulated, and a lead that is modulated with the amount of coverage
For further explanation of how to establish the audio files, check the guide in the pictorial that is found in this repository. 

## Alternative Actuation methods
We as authors trust that if you want to hook up a differing actuator, than a air pump, you have the relevant knowledge of pin declaration, and inclusion of triggering in the code. 

If you want the actuation to be triggered by use of the pads. You can use the switch case in the `void loop()` function
The switch case is placed below, this allows you to either ctrl+f it in the code to find it more quickly, or to understand it's function.
Case 0 to 3 speak for themselves as the cases that are active when a specific pad is covered the most. Case -1 is active when no pads are being triggered, also referred to as an 'idle' state
```
switch (activeMultiplexer) {
    case -1:
      {
        deactivateAllPumps();
        Serial.println("No multiplexer activated.");
        idleLED();
        ESNumber = -1;
      }
      break;
    case 0:
      {
        ColorAllLeds(255, 200, 0);
        volumeMap();
        activatePump(0, enA1, in2_1, 255, 2000, 2000);
        printActiveMulti(currentCaseTime, "0", coverage);
        ESNumber = 0;
      }
      break;
    case 1:
      {
        ColorAllLeds(50, 255, 0);
        volumeMap();
        activatePump(1, enA2, in1_2, 255, 5000, 2000);
        printActiveMulti(currentCaseTime, "1", coverage);
        ESNumber = 2;
      }
      break;
    case 2:
      {
        ColorAllLeds(173, 216, 230);
        volumeMap();
        activatePump(2, enA3, in1_3, 255, 1500, 4000);
        printActiveMulti(currentCaseTime, "2", coverage);
        ESNumber = 4;
      }
      break;
    case 3:
      {
        ColorAllLeds(210, 0, 0);
        volumeMap();
        int inf = random(100, 2500);
        int def = random(inf, inf + 500);
        activatePump(3, enA4, in1_4, 255, inf, def);
        printActiveMulti(currentCaseTime, "3", coverage);
        ESNumber = 6;
      }
      break;
}
```

## Additional Information:
Information page on this project: https://projects.id.tue.nl/demoday/7zILw3
