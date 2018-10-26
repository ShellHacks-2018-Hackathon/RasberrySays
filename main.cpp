int dormir(int i) {
    if (i == 1) {
        retrun 0;
    } else if (i ==2) {
        return 1;
    } else {
        return dormir(i-1) + dormir(i-2);
    }
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
  int counter = 0;
  int answerArray[35];
  int userAnswerArray[35];
  int turn = 0;
  int level = 5;

  int c;
  unsigned int portNumber = 9001;
  const unsigned int nBytes = 10240;

     // Showing User Random Color
  
    for (int x = 0; x < level; x++) {
          int answer = rand() % 4 + 1;
          answerArray[(level + x) - 1] = answer;
        for (int i = 0; i < bus.MatrixLeds(); i++) {
            image1d.leds[i].red = 0;
            image1d.leds[i].green = 0;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
        }
        everloop.Write(&image1d);
        usleep(50000);
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
        usleep(100000);
    }

        //Turning off Lights
    for (int i = 0; i < bus.MatrixLeds(); i++) {
      image1d.leds[i].red = 0;
      image1d.leds[i].green = 0;
      image1d.leds[i].blue = 0;
      image1d.leds[i].white = 0;
     }

    everloop.Write(&image1d);

        usleep(50000);

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
        connection_id = ac+++++-----cept(server_id, (struct sockaddr *)NULL, NULL);
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

//    while (turn != level) {
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
            image1d.leds[i].red = 100;
            image1d.leds[i].green = 0;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
            userAnswerArray[turn] = 1;
        } else if (i<=17) {
            image1d.leds[i].red = 0;
            image1d.leds[i].green = 100;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
            userAnswerArray[turn] = 2;
        } else if (i<=26) {
            image1d.leds[i].red = 0;
            image1d.leds[i].green = 0;
            image1d.leds[i].blue = 100;
            image1d.leds[i].white = 0;
            userAnswerArray[turn] = 3;
        } else {
            image1d.leds[i].red = 100;
            image1d.leds[i].green = 100;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
            userAnswerArray[turn] = 4;
        }
        turn += 1;
    	}
	}
    everloop.Write(&image1d); 
    //  } 

/*   
   bool userChoice = true;
   usleep(1000000);
   for (int i = 0; i  < level; i++) {
       if (userAnswerArray[i] != answerArray[i]) {
           userChoice = false;
       } 
   }

   if (!userChoice) {
       level = 1;
        // Flash to Red
        for (int i = 0; i < bus.MatrixLeds(); i++) {
     	if (((bus.MatrixLeds() - i) <= counter)) {
        	image1d.leds[i].red = 100;
         } else {
                image1d.leds[i].red = 0;
         }
            image1d.leds[i].green = 0;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
     }
     usleep(100000);
     everloop.Write(&image1d);
     if (counter == (bus.MatrixLeds() - 1)) {
        counter = 0;
      } else {
        counter++;
      }
   } */ 

    turn = 0;


}
}

lose () {
     for (int i = 0; i < 3; i++) {}
        flashRed()
        usleep(1500000)
        flashNoColor()
     }
     flashRed()
     usleep(1500000)

}

flashRed() {
    for (int i = 0; i < bus.MatrixLeds(); i++) {
      image1d.leds[i].red = 100;
      image1d.leds[i].green = 0;
      image1d.leds[i].blue = 0;
      image1d.leds[i].white = 0;
     }
     everloop.Write(&imageId);
}

flashNoColor() {
        for (int i = 0; i < bus.MatrixLeds(); i++) {
            image1d.leds[i].red = 0;
            image1d.leds[i].green = 0;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
        }
        everloop.Write(&imageId);
}

showScore() {
    int counter = 0
          for (int i = 0; i < bus.MatrixLeds(); i++) {
              if ((bus.MatrixLeds() - i <= counter) {
                image1d.leds[i].red = 100;
                } else {
                image1d.leds[i].red = 0;
                }
            image1d.leds[i].green = 0;
            image1d.leds[i].blue = 0;
            image1d.leds[i].white = 0;
        }
        usleep(100000)
        everloop.Write(&imageId);
        if (counter == (bus.MatrixLeds() - 1)) {
            counter = 0
        } else {
            counter++;
        }
}
