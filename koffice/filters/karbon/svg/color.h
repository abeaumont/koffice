
#define TORGB( red, green, blue ) \
{ \
	r = red; \
	b = blue; \
	g = green; \
}

void keywordToRGB( QString rgbColor, int &r, int &g, int &b )
{
	if( rgbColor == "aliceblue" )
		TORGB( 240, 248, 255)
	else if( rgbColor == "antiquewhite" )
		TORGB( 250, 235, 215)
	else if( rgbColor == "aqua" )
		TORGB( 0, 255, 255)
	else if( rgbColor == "aquamarine" )
		TORGB( 127, 255, 212 )
	else if( rgbColor == "azure" )
		TORGB( 240, 255, 255 )
	else if( rgbColor == "beige" )
		TORGB( 245, 245, 220 )
	else if( rgbColor == "bisque" )
		TORGB( 255, 228, 196 )
	else if( rgbColor == "black" )
		TORGB( 0, 0, 0 )
	else if( rgbColor == "blanchedalmond" )
		TORGB( 255, 235, 205 )
	else if( rgbColor == "blue" )
		TORGB( 0, 0, 255 )
	else if( rgbColor == "blueviolet" )
		TORGB( 138, 43, 226 )
	else if( rgbColor == "brown" )
		TORGB( 165, 42, 42 )
	else if( rgbColor == "burlywood" )
		TORGB( 222, 184, 135 )
	else if( rgbColor == "cadetblue" )
		TORGB( 95, 158, 160 )
	else if( rgbColor == "chartreuse" )
		TORGB( 127, 255, 0 )
	else if( rgbColor == "chocolate" )
		TORGB( 210, 105, 30 )
	else if( rgbColor == "coral" )
		TORGB( 255, 127, 80 )
	else if( rgbColor == "cornflowerblue" )
		TORGB( 100, 149, 237 )
	else if( rgbColor == "cornsilk" )
		TORGB( 255, 248, 220 )
	else if( rgbColor == "crimson" )
		TORGB( 220, 20, 60 )
	else if( rgbColor == "cyan" )
		TORGB( 0, 255, 255 )
	else if( rgbColor == "darkblue" )
		TORGB( 0, 0, 139 )
	else if( rgbColor == "darkcyan" )
		TORGB( 0, 139, 139 )
	else if( rgbColor == "darkgoldenrod" )
		TORGB( 184, 134, 11 )
	else if( rgbColor == "darkgray" )
		TORGB( 169, 169, 169 )
	else if( rgbColor == "darkgrey" )
		TORGB( 169, 169, 169 )
	else if( rgbColor == "darkgreen" )
		TORGB( 0, 100, 0 )
	else if( rgbColor == "darkkhaki" )
		TORGB( 189, 183, 107 )
	else if( rgbColor == "darkmagenta" )
		TORGB( 139, 0, 139 )
	else if( rgbColor == "darkolivegreen" )
		TORGB( 85, 107, 47 )
	else if( rgbColor == "darkorange" )
		TORGB( 255, 140, 0 )
	else if( rgbColor == "darkorchid" )
		TORGB( 153, 50, 204 )
	else if( rgbColor == "darkred" )
		TORGB( 139, 0, 0 )
	else if( rgbColor == "darksalmon" )
		TORGB( 233, 150, 122 )
	else if( rgbColor == "darkseagreen" )
		TORGB( 143, 188, 143 )
	else if( rgbColor == "darkslateblue" )
		TORGB( 72, 61, 139 )
	else if( rgbColor == "darkslategray" )
		TORGB( 47, 79, 79 )
	else if( rgbColor == "darkslategrey" )
		TORGB( 47, 79, 79 )
	else if( rgbColor == "darkturquoise" )
		TORGB( 0, 206, 209 )
	else if( rgbColor == "darkviolet" )
		TORGB( 148, 0, 211 )
	else if( rgbColor == "deeppink" )
		TORGB( 255, 20, 147 )
	else if( rgbColor == "deepskyblue" )
		TORGB( 0, 191, 255 )
	else if( rgbColor == "dimgray" )
		TORGB( 105, 105, 105 )
	else if( rgbColor == "dimgrey" )
		TORGB( 105, 105, 105 )
	else if( rgbColor == "dodgerblue" )
		TORGB( 30, 144, 255 )
	else if( rgbColor == "firebrick" )
		TORGB( 178, 34, 34 )
	else if( rgbColor == "floralwhite" )
		TORGB( 255, 250, 240 )
	else if( rgbColor == "forestgreen" )
		TORGB( 34, 139, 34 )
	else if( rgbColor == "fuchsia" )
		TORGB( 255, 0, 255 )
	else if( rgbColor == "gainsboro" )
		TORGB( 220, 220, 220 )
	else if( rgbColor == "ghostwhite" )
		TORGB( 248, 248, 255 )
	else if( rgbColor == "gold" )
		TORGB( 255, 215, 0 )
	else if( rgbColor == "goldenrod" )
		TORGB( 218, 165, 32 )
	else if( rgbColor == "gray" )
		TORGB( 128, 128, 128 )
	else if( rgbColor == "grey" )
		TORGB( 128, 128, 128 )
	else if( rgbColor == "green" )
		TORGB( 0, 128, 0 )
	else if( rgbColor == "greenyellow" )
		TORGB( 173, 255, 47 )
	else if( rgbColor == "honeydew" )
		TORGB( 240, 255, 240 )
	else if( rgbColor == "hotpink" )
		TORGB( 255, 105, 180 )
	else if( rgbColor == "indianred" )
		TORGB( 205, 92, 92 )
	else if( rgbColor == "indigo" )
		TORGB( 75, 0, 130 )
	else if( rgbColor == "ivory" )
		TORGB( 255, 255, 240 )
	else if( rgbColor == "khaki" )
		TORGB( 240, 230, 140 )
	else if( rgbColor == "lavender" )
		TORGB( 230, 230, 250 )
	else if( rgbColor == "lavenderblush" )
		TORGB( 255, 240, 245 )
	else if( rgbColor == "lawngreen" )
		TORGB( 124, 252, 0 )
	else if( rgbColor == "lemonchiffon" )
		TORGB( 255, 250, 205 )
	else if( rgbColor == "lightblue" )
		TORGB( 173, 216, 230 )
	else if( rgbColor == "lightcoral" )
		TORGB( 240, 128, 128 )
	else if( rgbColor == "lightcyan" )
		TORGB( 224, 255, 255 )
	else if( rgbColor == "lightgoldenrodyellow" )
		TORGB( 250, 250, 210 )
	else if( rgbColor == "lightgray" )
		TORGB( 211, 211, 211 )
	else if( rgbColor == "lightgrey" )
		TORGB( 211, 211, 211 )
	else if( rgbColor == "lightgreen" )
		TORGB( 144, 238, 144 )
	else if( rgbColor == "lightpink" )
		TORGB( 255, 182, 193 )
	else if( rgbColor == "lightsalmon" )
		TORGB( 255, 160, 122 )
	else if( rgbColor == "lightseagreen" )
		TORGB( 32, 178, 170 )
	else if( rgbColor == "lightskyblue" )
		TORGB( 135, 206, 250 )
	else if( rgbColor == "lightslategray" )
		TORGB( 119, 136, 153 )
	else if( rgbColor == "lightslategrey" )
		TORGB( 119, 136, 153 )
	else if( rgbColor == "lightsteelblue" )
		TORGB( 176, 196, 222 )
	else if( rgbColor == "lightyellow" )
		TORGB( 255, 255, 224 )
	else if( rgbColor == "lime" )
		TORGB( 0, 255, 0 )
	else if( rgbColor == "limegreen" )
		TORGB( 50, 205, 50 )
	else if( rgbColor == "linen" )
		TORGB( 250, 240, 230 )
	else if( rgbColor == "magenta" )
		TORGB( 255, 0, 255 )
	else if( rgbColor == "maroon" )
		TORGB( 128, 0, 0 )
	else if( rgbColor == "mediumaquamarine" )
		TORGB( 102, 205, 170 )
	else if( rgbColor == "mediumblue" )
		TORGB( 0, 0, 205 )
	else if( rgbColor == "mediumorchid" )
		TORGB( 186, 85, 211 )
	else if( rgbColor == "mediumpurple" )
		TORGB( 147, 112, 219 )
	else if( rgbColor == "mediumseagreen" )
		TORGB( 60, 179, 113 )
	else if( rgbColor == "mediumslateblue" )
		TORGB( 123, 104, 238 )
	else if( rgbColor == "mediumspringgreen" )
		TORGB( 0, 250, 154 )
	else if( rgbColor == "mediumturquoise" )
		TORGB( 72, 209, 204 )
	else if( rgbColor == "mediumvioletred" )
		TORGB( 199, 21, 133 )
	else if( rgbColor == "midnightblue" )
		TORGB( 25, 25, 112 )
	else if( rgbColor == "mintcream" )
		TORGB( 245, 255, 250 )
	else if( rgbColor == "mistyrose" )
		TORGB( 255, 228, 225 )
	else if( rgbColor == "moccasin" )
		TORGB( 255, 228, 181 )
	else if( rgbColor == "navajowhite" )
		TORGB( 255, 222, 173 )
	else if( rgbColor == "navy" )
		TORGB( 0, 0, 128 )
	else if( rgbColor == "oldlace" )
		TORGB( 253, 245, 230 )
	else if( rgbColor == "olive" )
		TORGB( 128, 128, 0 )
	else if( rgbColor == "olivedrab" )
		TORGB( 107, 142, 35 )
	else if( rgbColor == "orange" )
		TORGB( 255, 165, 0 )
	else if( rgbColor == "orangered" )
		TORGB( 255, 69, 0 )
	else if( rgbColor == "orchid" )
		TORGB( 218, 112, 214 )
	else if( rgbColor == "palegoldenrod" )
		TORGB( 238, 232, 170 )
	else if( rgbColor == "palegreen" )
		TORGB( 152, 251, 152 )
	else if( rgbColor == "paleturquoise" )
		TORGB( 175, 238, 238 )
	else if( rgbColor == "palevioletred" )
		TORGB( 219, 112, 147 )
	else if( rgbColor == "papayawhip" )
		TORGB( 255, 239, 213 )
	else if( rgbColor == "peachpuff" )
		TORGB( 255, 218, 185 )
	else if( rgbColor == "peru" )
		TORGB( 205, 133, 63 )
	else if( rgbColor == "pink" )
		TORGB( 255, 192, 203 )
	else if( rgbColor == "plum" )
		TORGB( 221, 160, 221 )
	else if( rgbColor == "powderblue" )
		TORGB( 176, 224, 230 )
	else if( rgbColor == "purple" )
		TORGB( 128, 0, 128 )
	else if( rgbColor == "red" )
		TORGB( 255, 0, 0 )
	else if( rgbColor == "rosybrown" )
		TORGB( 188, 143, 143 )
	else if( rgbColor == "royalblue" )
		TORGB( 65, 105, 225 )
	else if( rgbColor == "saddlebrown" )
		TORGB( 139, 69, 19 )
	else if( rgbColor == "salmon" )
		TORGB( 250, 128, 114 )
	else if( rgbColor == "sandybrown" )
		TORGB( 244, 164, 96 )
	else if( rgbColor == "seagreen" )
		TORGB( 46, 139, 87 )
	else if( rgbColor == "seashell" )
		TORGB( 255, 245, 238 )
	else if( rgbColor == "sienna" )
		TORGB( 160, 82, 45 )
	else if( rgbColor == "silver" )
		TORGB( 192, 192, 192 )
	else if( rgbColor == "skyblue" )
		TORGB( 135, 206, 235 )
	else if( rgbColor == "slateblue" )
		TORGB( 106, 90, 205 )
	else if( rgbColor == "slategray" )
		TORGB( 112, 128, 144 )
	else if( rgbColor == "slategrey" )
		TORGB( 112, 128, 144 )
	else if( rgbColor == "snow" )
		TORGB( 255, 250, 250 )
	else if( rgbColor == "springgreen" )
		TORGB( 0, 255, 127 )
	else if( rgbColor == "steelblue" )
		TORGB( 70, 130, 180 )
	else if( rgbColor == "tan" )
		TORGB( 210, 180, 140 )
	else if( rgbColor == "teal" )
		TORGB( 0, 128, 128 )
	else if( rgbColor == "thistle" )
		TORGB( 216, 191, 216 )
	else if( rgbColor == "tomato" )
		TORGB( 255, 99, 71 )
	else if( rgbColor == "turquoise" )
		TORGB( 64, 224, 208 )
	else if( rgbColor == "violet" )
		TORGB( 238, 130, 238 )
	else if( rgbColor == "wheat" )
		TORGB( 245, 222, 179 )
	else if( rgbColor == "white" )
		TORGB( 255, 255, 255 )
	else if( rgbColor == "whitesmoke" )
		TORGB( 245, 245, 245 )
	else if( rgbColor == "yellow" )
		TORGB( 255, 255, 0 )
	else if( rgbColor == "yellowgreen" )
		TORGB( 154, 205, 50 )
}

