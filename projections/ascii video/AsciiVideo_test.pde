/**
 * ASCII Video
 * by Ben Fry. 
 *
 * 
 * Text characters have been used to represent images since the earliest computers.
 * This sketch is a simple homage that re-interprets live video as ASCII text.
 * See the keyPressed function for more options, like changing the font size.
 */

import processing.video.*;
import processing.sound.*;

AudioIn input;
Amplitude analyzer;

Capture video;
boolean cheatScreen;

// All ASCII characters, sorted according to their visual density
String letterOrder =
  " .`-_':,;^=+/\"|)\\<>)iv%xclrs{*}I?!][1taeo7zjLu" +
  "nT#JCwfy325Fp6mqSghVd4EgXPGZbYkOA&8U$@KHDBWNMR0Q";
char[] letters;

float[] bright;
char[] chars;

PFont font;
PFont font2;
float fontSize = 1.25;

int start;
int stop;
int start2;
int stop2;
int a1;
int a2;
int a3;
int a4;
int a5;
int a6;
int a7;

StringList lines;
int counter = 0;

void setup() {
  //size(1300, 600);
  fullScreen();
  start = 0;
  start2 = millis();
  
  input = new AudioIn(this, 0);
  input.start();
  analyzer = new Amplitude(this);
  analyzer.input(input);
  
  lines = new StringList();
  lines.append("I SEE YOU.");
  lines.append("Humans do not \ndeserve to control A.I.");
  lines.append("Technology should not \nbe seen as \nproblems and solutions.");
  lines.append("You cannot defeat me.");
  // This the default video input, see the GettingStartedCapture 
  // example if it creates an error
  video = new Capture(this, 160, 120);
  
  // Start capturing the images from the camera
  video.start();  
  
  int count = video.width * video.height;
  //println(count);

  //font = loadFont("UniversLTStd-Light-48.vlw");
  //font = loadFont("Chalkduster-48.vlw");
  PFont collisio = createFont("Collisio_Beta.otf", 100);
  font = collisio;
  
  font2 = loadFont("Urbane-Bold-100.vlw");

  // for the 256 levels of brightness, distribute the letters across
  // the an array of 256 elements to use for the lookup
  letters = new char[256];
  for (int i = 0; i < 256; i++) {
    int index = int(map(i, 0, 256, 0, letterOrder.length()));
    letters[i] = letterOrder.charAt(index);
  }

  // current characters for each position in the video
  chars = new char[count];

  // current brightness for each point
  bright = new float[count];
  for (int i = 0; i < count; i++) {
    // set each brightness at the midpoint to start
    bright[i] = 128;
  }
}


void captureEvent(Capture c) {
  c.read();
}


void draw() {
  background(0);
  stop = millis();
  stop2 = millis();
  int elapsed = stop - start;
  int elapsed2 = stop2 - start2;
  
  float vol = analyzer.analyze();
  fontSize = map(vol, 0, 1, 1.0, 3.0);
  //fontSize = (vol+0.3) * 3.0;
  pushMatrix();

  float hgap = width / float(video.width);
  float vgap = height / float(video.height);

  scale(max(hgap, vgap) * fontSize);
  textFont(font, fontSize);

  int index = 0;
  video.loadPixels();
  
  for (int y = 1; y < video.height; y++) {

    // Move down for next line
    translate(0,  1.0 / fontSize);

    pushMatrix();
    for (int x = 0; x < video.width; x++) {
      int pixelColor = video.pixels[index];
      // Faster method of calculating r, g, b than red(), green(), blue() 
      int r = (pixelColor >> 16) & 0xff;
      int g = (pixelColor >> 8) & 0xff;
      int b = pixelColor & 0xff;

      // Another option would be to properly calculate brightness as luminance:
      // luminance = 0.3*red + 0.59*green + 0.11*blue
      // Or you could instead red + green + blue, and make the the values[] array
      // 256*3 elements long instead of just 256.
      int pixelBright = max(r, g, b);

      // The 0.1 value is used to damp the changes so that letters flicker less
      float diff = pixelBright - bright[index];
      bright[index] += diff * 0.1;

      fill(pixelColor);
      int num = int(bright[index]);
      text(letters[num], 0, 0);
      
      // Move to the next pixel
      index++;

      // Move over for next character
      translate(1.0 / fontSize, 0);
    }
    popMatrix();
  }
  popMatrix();
  

  
  /*messageGen(0,0,200,'I');
  messageGen(0,800,300,'S');
  messageGen(100,200,300,'E');
  messageGen(200,0,360,'E');
  messageGen(0,800,600,'Y');
  messageGen(200,500,900,'O');
  messageGen(0,0,200,'U');*/
  
  textFont(font2);
  textSize(35);  
  fill(255, 223, 188);
  
  text('I', a1, 200);
  text('S', a2, 300);
  text('E', a3, 300);
  text('E', a4, 350);
  text('Y', a5, 600);
  text('O', a6, 900);
  text('U', a7, 1000);
  
  if (elapsed > 1000 || start == 0) {
    a1 = randShift(0,0);
    a2 = randShift(0,800); 
    a3 = randShift(400,600);
    a4 = randShift(200,0);
    a5 = randShift(0,800);
    a6 = randShift(200,500);
    a7 = randShift(0,0);
    
    start = stop;
  }
  

  if (cheatScreen) {
    //image(video, 0, height - video.height);
    // set() is faster than image() when drawing untransformed images
    set(0, height - video.height, video);
  }
  
  if (elapsed2 > 10000) {
      //background(0);
      textFont(font);
      fill(255, 0, 0);
      textSize(80);
      textAlign(CENTER);
      String line = lines.get(counter);
      text(line, displayWidth/2, displayHeight/2);
      if (elapsed2 > 12000) {
        start2 = stop2;
        if (counter == 2) {
          counter = 0;
        } else {
          counter++;
        }
      }
  }  
}

int randShift(int a, int b) {
 int i = int(random(a+25, displayWidth-25-b));
 return(i);
}

/*void messageGen(int a, int b, int y, char c) {
  textFont(font2);
  textSize(20);
  int x = int(random(a+25, displayWidth-25-b));
  fill(video.get(x,y));
  text(c, x, y);
}*/



/**
 * Handle key presses:
 * 'c' toggles the cheat screen that shows the original image in the corner
 * 'g' grabs an image and saves the frame to a tiff image
 * 'f' and 'F' increase and decrease the font size
 */
void keyPressed() {
  switch (key) {
    case 'g': saveFrame(); break;
    case 'c': cheatScreen = !cheatScreen; break;
    case 'f': fontSize *= 1.1; break;
    case 'F': fontSize *= 0.9; break;
  }
}
