// uDroplet.pde Processing sketch
// http://www.charleslabs.fr/en/project-Microfluidics+platform

// Load sketch into Processing, available from:
// https://processing.org/

import processing.serial.*;
import javax.swing.JOptionPane;

// Settings
final int X_ROWS = 8, Y_COLS = 8;
final int SERIAL_SPEED = 115200;

// Vars
Serial serial;
Button[][] buttons;
int w=0, h=0;
String serialString;

void setup() {
  size(512, 512);
  surface.setTitle("µDroplet GUI");
  // Serial Port
  try {
    if (Serial.list().length == 0) {
      println("No serial device connected");
      exit();
    }
    else if (Serial.list().length == 1) {
      // only one device, select it (or try...)
      serial = new Serial(this, Serial.list()[0], SERIAL_SPEED);
    }
    else {
      // more than 1, show dialog
      String[] ports = Serial.list();
      
      String selection = (String) JOptionPane.showInputDialog(null,
          "Select the serial port that corresponds to the uDroplet:",
          "Select serial port",
          JOptionPane.PLAIN_MESSAGE,
          null,
          ports,
          ports[ports.length-1]);
      
      if (selection == null || selection.isEmpty()) 
        exit();

      serial = new Serial(this, selection, SERIAL_SPEED);
    }
  }
  catch (Exception e) { 
    println("Not able to connect to serialPort (error:"+e.getClass().getName()+" " + e.getMessage() + ")");
    exit();
  }
  
  // Buttons setup
  surface.setResizable(true);
  buttons = new Button[X_ROWS][Y_COLS];
  for (int x = 0; x < X_ROWS; x++) {
    for (int y = 0; y < Y_COLS; y++) {
      buttons[x][y] = new Button();
    }
  }
  windowResized();
}

void draw() {
  if (w != width || h != height) {
    w=width ;
    h=height ;
    windowResized();
  }
  
  // Read Serial Port (if we can)
  while (serial.available() > 0) {
    serialString = serial.readStringUntil('\n');
    if (serialString != null) {
      print("->"+serialString);
      String[] coordinates = split(serialString.replace("\n", ""), ',');
      if (coordinates.length == 3) {
        int x = int(coordinates[0]);
        int y = int(coordinates[1]);
        int state = int(coordinates[2]);
        if( x>=0 && x<X_ROWS && y>=0 && y<=Y_COLS)
          buttons[x][y].setState(state!=0);
      }
    }
    else continue;
  }
  
  // Draw
  background(#222222);
  for (int x = 0; x < X_ROWS; x++) {
    for (int y = 0; y < Y_COLS; y++) {
      buttons[x][y].draw();
    }
  }
}

class Button {
  private boolean active = false;
  private int x_pos, y_pos; // px
  private int x_size, y_size; // px
  
  Button() {}

  boolean isActive() {
    return this.active;
  }

  void reDim(int x, int y, int x_size, int y_size) {
    this.x_pos = x;
    this.y_pos = y;
    this.x_size = x_size;
    this.y_size = y_size;
  }

  void draw() {
    strokeWeight(2);
    stroke(#222222);

    boolean mouseHover = this.mouseOver();

    // Cell color
    if (mouseHover && this.active) 
      fill(#529fd6);
    else if (mouseHover && !this.active)
      fill(225);
    else if (!mouseHover && this.active)
      fill(#72bff6);
    else
      fill(255);

    rect(x_pos, y_pos, x_size, y_size, x_size*.1f);
  }
  
  void setState(boolean newState){
    this.active = newState;
  }
  
  void toggle(int x, int y) {
    String cmd = x+","+y+","+int(!this.active)+"\n";
    serial.write(cmd);
    print("<-"+cmd);
  }

  boolean mouseOver() {
    if (mouseX>=x_pos && mouseX < x_pos+x_size && mouseY>=y_pos && mouseY <y_pos+y_size)
      return true;
    return false;
  }
}

// Click on a cell
void mousePressed() {
  for (int x = 0; x < X_ROWS; x++) {
    for (int y = 0; y < Y_COLS; y++) {
      if (buttons[x][y].mouseOver()) {
        buttons[x][y].toggle(x,y); 
        return;
      }
    }
  }
}

// Move cells with arrow key
// FIXME : does not work with serial connection
void keyPressed() {
    /*if (keyCode == UP || keyCode == LEFT) {
      for (int x = 0; x < X_ROWS; x++) {
        for (int y = 0; y < Y_COLS; y++) {
          if (buttons[x][y].isActive()) {
            buttons[x][y].toggle(x,y); 
            if ((keyCode == LEFT && x>0) || (keyCode == UP && y>0))
              buttons[x - (keyCode == LEFT?1:0)][y - (keyCode == UP?1:0)].toggle(x,y);
          }
        }
      }
    } else if (keyCode == DOWN || keyCode == RIGHT) {
      for (int x = X_ROWS - 1; x >= 0; x--) {
        for (int y = Y_COLS - 1; y >= 0; y--) {
          if (buttons[x][y].isActive()) {
            buttons[x][y].toggle(x,y); 
            if ((keyCode == RIGHT && x<X_ROWS - 1) || (keyCode == DOWN && y<Y_COLS - 1))
              buttons[x + (keyCode == RIGHT?1:0)][y + (keyCode == DOWN?1:0)].toggle(x,y);
          }
        }
      }
    }*/
    if(key == BACKSPACE || key == DELETE){
      for (int x = 0; x < X_ROWS; x++)
        for (int y = 0; y < Y_COLS; y++)
          if (buttons[x][y].isActive())
            buttons[x][y].toggle(x,y);
    }
}

// On window resize, compute UI size
void windowResized() {
  int x_size = (int)((float)width/X_ROWS);
  int y_size = (int)((float)height/Y_COLS);
  for (int x = 0; x < X_ROWS; x++) {
    for (int y = 0; y < Y_COLS; y++) {
      buttons[x][y].reDim(x*x_size, y*y_size, x_size, y_size);
    }
  }
}
