char *topstring[16];
char *botstring[16];

void ScrollString(char *tstring, char *bstring) {
  int i;
  char ttemp, btemp;

  int tstringlen = strlen(tstring);
  int bstringlen = strlen(bstring)

  // initialize top and bot strings to be spaces
  for (int i=0; i<16; i++) {
    topstring[i] = " ";
    botstring[i] = " ";
  }

  //command(set cursor 1st bit 1st line);
	WriteString(tstring);

  // first bit 2nd line
  command(0xC0UL);

  WriteString(bstring);

	// fill char array of first string
	for (i=0; i<tstringlen; i++)
		topstring[i] = topstring[i];

	// fill char array of second string
	for (i=0; i<botstringlen; i++)
		botstring[i] = botstring[i];

  i = 0, j = 0;

  while (1) {

    // add a delay here


    ttemp = topstring[i];
    btemp = botstring[j];

    // shift all characters on line 1 and 2 to the left by 1
    for (int a=1; a<16; a++) {
      topstring[a-1] = topstring[a];
      botstring[a-1] = topstring[a];
    }

    // move the original first bit to the last bit
    topstring[15] = ttemp;
    botstring[15] = btemp;

    // clear screen
    command(0x1UL); 
    
    // output top string to the LCD
    WriteString(topstring);

    // move cursor to 1st bit 2nd line
    command(0xC0UL);

    // output bot string to the LCD
    WriteString(botstring);

    i = 0;
  }
}