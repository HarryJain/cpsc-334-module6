Circle c1;
Circle c2;
Circle c3;

int radius = 500;


void setup()
{
  background(0);
  size(displayWidth, displayHeight);
  c1 = new Circle(color(255,0,0),(int)random(100, (int)(displayWidth * 0.75)), (int)random(100, (int)(displayHeight * 0.75)), (int)random(500,1000), 100.);
  c2 = new Circle(color(0,0,255), (int)random(100, (int)(displayWidth * 0.75)), (int)random(100, (int)(displayHeight * 0.75)), (int)random(500,1000), 200.);
  c3 = new Circle(color(0,255,0), (int)random(100, (int)(displayWidth * 0.75)), (int)random(100, (int)(displayHeight * 0.75)), (int)random(500,1000), 300.);
 
  
  
}

void draw()
{
  

  blendMode(ADD);
  c1.shift();
  c1.display();
  c2.shift();
  c2.display();
  c3.shift();
  c3.display();
    filter(BLUR, 20);
    
  blendMode(SUBTRACT);
  PImage img = loadImage("akw1.jpg");
  img.resize(width, height);
  image(img, 0, 0);
  //fill(255);
  //textSize(18);
  //text("there is no solution", (int)random(100, (int)(displayWidth * 0.75)), (int)random(100, (int)(displayHeight * 0.75))); 

  

}

class Circle {
  color c;
  float xpos;
  float ypos;
  int rad;
  float speed;
  int xdirection = 1;  // Left or Right
  int ydirection = 1;  // Top to Bottom

  // The Constructor is defined with arguments.
  Circle(color tempC, float tempXpos, float tempYpos, int tempXradius, float tempSpeed) {
    c = tempC;
    xpos = tempXpos;
    ypos = tempYpos;
    rad = tempXradius;
    speed = tempSpeed;
  }

  void display() {
    noStroke();
    fill(c);
    //rectMode(CENTER);
    //rect(xpos,ypos,20,10);
    circle(xpos, ypos, rad);
  }

  void shift() {
    xpos = (int)random(0, displayWidth);
    ypos = (int)random(0, displayHeight);
    //xpos = xpos + speed * xdirection;
    //ypos = ypos + speed * ydirection;
  //  if (xpos >= width-rad || xpos <= rad) {
  //  xdirection *= (-1 * random(0.5,1));
  //  }
  //  if (ypos >= height-rad || ypos <= rad) {
  //    ydirection *= (-1 * random(0.5,1));
  //  }
  }
}
