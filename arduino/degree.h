const uint8_t DegreeBitmaps[] PROGMEM = {
//00 °
/*| 8 4 2 1 8 4 2 1 8 4 2 1 8 4 2 1 |*/
/*| . . . . . . X X X . . . . . . . |*/  0x03,0x80,
/*| . . . . . X . , . X . . . . . . |*/  0x04,0x40,
/*| . . . . X . . , . . X . . . . . |*/  0x08,0x20,
/*| . . . . . X . , . X . . . . . . |*/  0x04,0x40,
/*| . . . . . . X X X . . . . . . . |*/  0x03,0x80,
0x00};//One more byte just in case

const GFXglyph DegreeGlyphs[] PROGMEM = {
  //Index,  W, H, xAdv, dX, dY
  {     0, 16, 5, 14, 1, -12}}; //00 °
  //Index,  W, H, xAdv, dX, dY
const GFXfont Degree PROGMEM = {
  (uint8_t  *)DegreeBitmaps,
  (GFXglyph *)DegreeGlyphs,
  0, 0, 24 //ASCII start, ASCII stop,y Advance
};
