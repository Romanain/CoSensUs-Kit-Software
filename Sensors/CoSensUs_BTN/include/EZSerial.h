String inputString = ""; // a String to hold incoming data
String lastSerialMessage = "";
bool messageComplete = false; // whether the string is complete

// Easy serial print code template.
// I once found this on the stackoverflow some years ago and have subsequently used it in multiple projects
// Unfortunately I do not remember the original author, and am therefore not able to credit them.
template <typename T>
Print &operator<<(Print &printer, T value)
{
  printer.print(value);
  return printer;
}

// Listen to Serial port
void serialEvent()
{
  if (Serial.available())
  {
    char inChar = (char)Serial.read();
    inputString += inChar;

    if (inChar == '\n')
    {
      messageComplete = true;
      lastSerialMessage = inputString;
      inputString = "";
    }
  }
}
