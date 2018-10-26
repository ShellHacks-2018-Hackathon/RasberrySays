#include <json.h>
#include <math.h>
#include <matrix_hal/everloop.h>
#include <matrix_hal/everloop_image.h>
#include <matrix_hal/matrixio_bus.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <array>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

namespace hal = matrix_hal;

// ENERGY_COUNT : Number of sound energy slots to maintain.
#define ENERGY_COUNT 36
// MAX_VALUE : controls smoothness
#define MAX_VALUE 200
// INCREMENT : controls sensitivity
#define INCREMENT 20
// DECREMENT : controls delay in the dimming
#define DECREMENT 1
// MAX_BRIGHTNESS: Filters out low energy
#define MIN_THRESHOLD 16
// MAX_BRIGHTNESS: 0 - 255
#define MAX_BRIGHTNESS 50

double x, y, z, E;
int timestamp = 0;
int levelOffset = 0;
int counterA = 0;
int counterB = 0;
int energy_array[ENERGY_COUNT];
int states[7] = {1,0,0,0,0,0,0};
int currentState = 0;
int nextState = 0; 
int timeToWakeUp = 0;
int counter = 0;
int answerArray[35];
int userAnswerArray[35];
int turns = 1;
int userturns = 1;
int level = 1;
const double leds_angle_mcreator[35] = {
    170, 159, 149, 139, 129, 118, 108, 98,  87,  77,  67,  57,
    46,  36,  26,  15,  5,   355, 345, 334, 324, 314, 303, 293,
    283, 273, 262, 252, 242, 231, 221, 211, 201, 190, 180};

const double led_angles_mvoice[18] = {170, 150, 130, 110, 90,  70,
                                      50,  30,  10,  350, 330, 310,
                                      290, 270, 250, 230, 210, 190};

void increase_pots() {
  // Convert x,y to angle. TODO: See why x axis from ODAS is inverted
  double angle_xy = fmodf((atan2(y, x) * (180.0 / M_PI)) + 360, 360);
  // Convert angle to index
  int i_angle = angle_xy / 360 * ENERGY_COUNT;  // convert degrees to index
  // Set energy for this angle
  energy_array[i_angle] += INCREMENT * E;
  // Set limit at MAX_VALUE
  energy_array[i_angle] =
      energy_array[i_angle] > MAX_VALUE ? MAX_VALUE : energy_array[i_angle];
}

void decrease_pots() {
  for (int i = 0; i < ENERGY_COUNT; i++) {
    energy_array[i] -= (energy_array[i] > 0) ? DECREMENT : 0;
  }
}

void json_parse_array(json_object *jobj, char *key) {
  // Forward Declaration
  void json_parse(json_object * jobj);
  enum json_type type;
  json_object *jarray = jobj;
  if (key) {
    if (json_object_object_get_ex(jobj, key, &jarray) == false) {
      printf("Error parsing json object\n");
      return;
    }
  }

  int arraylen = json_object_array_length(jarray);
  int i;
  json_object *jvalue;

  for (i = 0; i < arraylen; i++) {
    jvalue = json_object_array_get_idx(jarray, i);
    type = json_object_get_type(jvalue);

    if (type == json_type_array) {
      json_parse_array(jvalue, NULL);
    } else if (type != json_type_object) {
    } else {
      json_parse(jvalue);
    }
  }
}

void json_parse(json_object *jobj) {
  enum json_type type;
  unsigned int count = 0;
  decrease_pots();
  json_object_object_foreach(jobj, key, val) {
    type = json_object_get_type(val);
    switch (type) {
      case json_type_boolean:
        break;
      case json_type_double:
        if (!strcmp(key, "x")) {
          x = json_object_get_double(val);
        } else if (!strcmp(key, "y")) {
          y = json_object_get_double(val);
        } else if (!strcmp(key, "z")) {
          z = json_object_get_double(val);
        } else if (!strcmp(key, "E")) {
          E = json_object_get_double(val);
        }
        increase_pots();
        count++;
        break;
      case json_type_int:
	timestamp = json_object_get_int(val);	
        break;
      case json_type_string:
        break;
      case json_type_object:
        if (json_object_object_get_ex(jobj, key, &jobj) == false) {
          printf("Error parsing json object\n");
          return;
        }
        json_parse(jobj);
        break;
      case json_type_array:
        json_parse_array(jobj, key);
        break;
    }
  }
}

void simonOrUserInput() {
    if (turns < level) {
        states[1] = 1;
        states[currentState] = 0;
        timeToWakeUp = timestamp + 300;
        nextState = 0;
        turns++;
    } else {
        states[2] = 1;
        states[currentState] = 0;
        timeToWakeUp = timestamp + 300;
        nextState = 3;
    }
}

void userInputOrCheckAnswer() {
    if (userturns < level) {
        states[6] = 1;
        states[currentState] = 0;
        timeToWakeUp = timestamp + 300;
        nextState = 3;
        userturns++;
    } else {
        states[6] = 1;
        states[currentState] = 0;
        timeToWakeUp = timestamp + 300;
        nextState = 4;
    }
}

void goToNextLevel() {
    level++;
    userturns = 1;
    turns = 1;
    states[currentState] = 0;
    states[0] = 1;
    nextState = 1; 
}

void GameOver() {
    level = 1;
    turns = 1;
    userturns = 1;
    states[currentState] = 0;
    states[6] = 1;
    nextState = 0;
    timeToWakeUp = timestamp + 1000; 
}

int main(int argc, char *argv[]) {
  // Everloop Initialization
  hal::MatrixIOBus bus;
  if (!bus.Init()) return false;
  hal::EverloopImage image1d(bus.MatrixLeds());
  hal::Everloop everloop;
  everloop.Setup(&bus);

  // Clear all LEDs
  for (hal::LedValue &led : image1d.leds) {
    led.red = 0;
    led.green = 100;
    led.blue = 0;
    led.white = 0;
  }
  everloop.Write(&image1d);

  char verbose = 0x00;

  int server_id;
  struct sockaddr_in server_address;
  int connection_id;
  char *message;
  int messageSize;


  int c;
  unsigned int portNumber = 9001;
  const unsigned int nBytes = 10240;

     // Showing User Random Color
  
	 srand (time(NULL));
        //turnsing off Lights

        // Comnecting

        server_id = socket(AF_INET, SOCK_STREAM, 0);

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(portNumber);

        printf("Binding socket........... ");
        fflush(stdout);
        bind(server_id, (struct sockaddr *)&server_address, sizeof(server_address));
        printf("[OK]\n");

        printf("Listening socket......... ");
        fflush(stdout);
        listen(server_id, 1);
        printf("[OK]\n");

        printf("Waiting for connection in port %d ... ", portNumber);
        fflush(stdout);
        connection_id = accept(server_id, (struct sockaddr *)NULL, NULL);
        printf("[OK]\n");

        message = (char *)malloc(sizeof(char) * nBytes);

        printf("Receiving data........... \n\n");


  while ((messageSize = recv(connection_id, message, nBytes, 0)) > 0) {
    message[messageSize] = 0x00;
// while (true) {
	

 
    // if ((messageSize = recv(connection_id, message, nBytes, 0)) > 0) {
    // message[messageSize] = 0x00;


    //Waiting for user response

    printf("Inside while loop message: %s\n\n", message);
    json_object *jobj = json_tokener_parse(message);
    json_parse(jobj);

    printf("TimeStamp Value: %d\n", timestamp);
    int timeStampCompare = timestamp;

	if (states[0] == 1){
        currentState = 0;
         	int answer = (rand() % 4) + 1;
          	answerArray[turns - 1] = answer;
		
        	if (answer == 1) {
            	   for (int i = 0; i <= 8; i++) {
                	image1d.leds[i].red = 100;
	                image1d.leds[i].green = 0;
	                image1d.leds[i].blue = 0;
	                image1d.leds[i].white = 0;
        	    }
	        } else if (answer == 2) {
        	    for (int i = 9; i <= 17; i++) {
                	image1d.leds[i].red = 0;
	                image1d.leds[i].green = 100;
        	        image1d.leds[i].blue = 0;
	                image1d.leds[i].white = 0;
	            }
       		} else if (answer == 3) {
                    for (int i = 18; i <= 26; i++) {
                        image1d.leds[i].red = 0;
                        image1d.leds[i].green = 0;
                        image1d.leds[i].blue = 100;
                        image1d.leds[i].white = 0;
                     }
        	} else if (answer == 4) {
            	    for (int i = 27; i <= 34; i++) {
                	image1d.leds[i].red = 100;
              		image1d.leds[i].green = 100;
                	image1d.leds[i].blue = 0;
                	image1d.leds[i].white = 0;
                 	}
   	        }

        	everloop.Write(&image1d);

            // Go to Wait Function
            states[currentState] = 0;
            states[1] = 1;
            nextState = 2;
            timeToWakeUp = timestamp + 300;
	} else if (states[1] == 1) {
        currentState = 1;
        if (timeToWakeUp <= timestamp) {
           simonOrUserInput();
        }
    } else if (states[2] == 1) {
        currentState = 2;
	     for (int i = 0; i < bus.MatrixLeds(); i++) {
      		image1d.leds[i].red = 0;
      		image1d.leds[i].green = 0;
      		image1d.leds[i].blue = 0;
      		image1d.leds[i].white = 0;
     }

        everloop.Write(&image1d);

        states[currentState] = 0;
        states[nextState] = 1;


	} else  if (states[3] == 1) {
        currentState = 3;

    for (int i = 0; i < bus.MatrixLeds(); i++) {
      // led index to angle
		
      int led_angle = bus.MatrixName() == hal::kMatrixCreator
                          ? leds_angle_mcreator[i]
                          : led_angles_mvoice[i];
      // Convert from angle to pots index
      int index_pots = led_angle * ENERGY_COUNT / 360;
      // Mapping from pots values to color
      int color = energy_array[index_pots] * MAX_BRIGHTNESS / MAX_VALUE;

      // Removing colors below the threshold
      if (color < MIN_THRESHOLD) {
      	image1d.leds[i].red = 0;
      	image1d.leds[i].green = 0;
      	image1d.leds[i].blue = 0;
      	image1d.leds[i].white = 0;
      } else {
	// Make Red
        if (i<=8) {
		for (int j = 0; j <= 8; j++) {
              image1d.leds[j].red = 100;
              image1d.leds[j].green = 0;
              image1d.leds[j].blue = 0;
              image1d.leds[j].white = 0;      
		} 
            everloop.Write(&image1d);           
            userAnswerArray[userturns-1] = 1;
        } else if (i<=17) {
		for(int j = 9; j <= 17; j++){
            image1d.leds[j].red = 0;
            image1d.leds[j].green = 100;
            image1d.leds[j].blue = 0;
            image1d.leds[j].white = 0;
		}
	    everloop.Write(&image1d);
        userAnswerArray[userturns-1] = 2;
        } else if (i<=26) {
		for(int j = 18; j <= 26; j++){
            image1d.leds[j].red = 0;
            image1d.leds[j].green = 0;
	        image1d.leds[j].blue = 100;
            image1d.leds[j].white = 0;
		}
	    everloop.Write(&image1d);
        userAnswerArray[userturns-1] = 3;
        } else {
		for(int j = 27; j <= 35; j++){
            image1d.leds[j].red = 100;
            image1d.leds[j].green = 100;
            image1d.leds[j].blue = 0;
            image1d.leds[j].white = 0;
		}
	    everloop.Write(&image1d);
        userAnswerArray[userturns-1] = 4;
        }

    }
	}
    
    userInputOrCheckAnswer();

    } else if (states[4] == 1) {
        currentState = 4;
        for (int i = 0; i < turns; i++) {
            if (userAnswerArray[i] != answerArray[i]) {
                // GameOver
                states[currentState] = 0;
                states[5] = 1;
            }
        }

        if (states[5] != 1) {
            goToNextLevel();
        }
    } else if (states[5] == 1) {
        currentState = 5;
         for (int i = 0; i < bus.MatrixLeds(); i++) {
      		image1d.leds[i].red = 255;
      		image1d.leds[i].green = 0;
      		image1d.leds[i].blue = 0;
      		image1d.leds[i].white = 0;
        }
        everloop.Write(&image1d);
        GameOver();
    } else if (states[6] == 1) {
        currentState = 6;
        if (timeToWakeUp <= timestamp) {
           states[currentState] = 0;
           states[nextState] = 1;
        }
    }
    for (int i = 0; i < 7; i++) { 
    printf("State %d = %d /t", i, states[i]);
}
}




}