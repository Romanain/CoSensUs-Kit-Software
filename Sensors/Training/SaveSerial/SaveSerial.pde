import interfascia.*;
import processing.serial.*;

Serial COMPort; 

PFont f1, f2;
String[] lines = new String[0];
String lastRead = "";
boolean firstLine = true;
boolean recording = false;
boolean stopRecording = false;
boolean startRecording = false;
String label = "Unspecified";
String labelInput;

GUIController c;
IFButton b1, b2, b3, b4;
IFTextField t;
IFLookAndFeel defaultLook;

PrintWriter output;

void setup() 
{
  String portName = Serial.list()[0];
  COMPort = new Serial(this, portName, 115200);

  size(640, 360);

  f1 = createFont("Courier New Bold", 16);
  f2 = createFont("Courier New Bold", 10);

  textFont(f1);
  c = new GUIController (this);

  defaultLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  defaultLook.baseColor = color(40);
  defaultLook.textColor = color(200);
  defaultLook.highlightColor = color(255, 204, 0);

  b1 = new IFButton ("Start recording", 40, 65, 120, 80);
  b2 = new IFButton ("Pause recording", 180, 65, 120, 80);
  b3 = new IFButton ("Exit and save", 320, 65, 120, 80);
  b4 = new IFButton ("Set condition", 215, 160, 160, 21);
  t = new IFTextField("Text Field", 40, 160, 160);

  b1.addActionListener(this);
  b2.addActionListener(this);
  b3.addActionListener(this);
  b4.addActionListener(this);
  t.addActionListener(this);

  c.setLookAndFeel(defaultLook);

  c.add(b1);
  c.add(b2);
  c.add(b3);
  c.add(b4);
  c.add(t);

  output = createWriter("SensorRead.txt");
}

int lineCounter = 0;

void draw() 
{
  background(40);

  if (COMPort.available() > 0) { 

    String read = COMPort.readString(); 
    lastRead = read;

    if (!firstLine) {
      lines = append(lines, read);

      if (recording) {
        output.print(read);
        if (read.indexOf("\n")>0) {
          lineCounter++;
        }
        if (stopRecording) {
          if (read.indexOf("}")>0) {
            recording = false;
          }
        }
      } else if (startRecording) { 
        if (read.indexOf("Condition")>0) {
          recording = true;
          output.print(read);
        }
      }
    } else {
      if (read.indexOf("{")==0) {
        firstLine = false;
      }
    }
  }

  textFont(f1);
  fill(255, 255, 255);
  text("Data points: ", 40, 45);
  fill(255, 204, 0);
  text(lineCounter, 180, 45);
  if (recording) {
    if (lineCounter < 10) {
      text("(Recording condition '" + label + "')", 210, 45);
    } else if (lineCounter < 100) {
      text("(Recording condition '" + label + "')", 218, 45);
    } else if (lineCounter < 1000) {
      text("(Recording condition '" + label + "')", 226, 45);
    } else if (lineCounter < 10000) {
      text("(Recording condition '" + label + "')", 234, 45);
    } else if (lineCounter < 100000) {
      text("(Recording condition '" + label + "')", 242, 45);
    }
  }

  textFont(f2);
  fill(200, 200, 200);
  text(lastRead, 40, 320);

  fill(255, 255, 255);
  text("Current label: ", 40, 200);
  fill(255, 204, 0);
  text(label, 140, 200);
}

void actionPerformed (GUIEvent e) {
  if (e.getSource() == b1) {
    startRecording = true;
  } else if (e.getSource() == b2) {
    output.flush(); 
    stopRecording = true;
  } else if (e.getSource() == b3) {
    println("End of program");
    output.close(); // Finishes the file
    exit(); // Stops the program
  } else if (e.getSource() == b4) {
    label = labelInput;
    println(label);
    COMPort.write(label);
  } else if (e.getMessage() != "") {
    labelInput = t.getValue();
  }
}
