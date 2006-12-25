/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

KFORMULA_NAMESPACE_BEGIN

struct UnicodeNameTable { short unicode; const char* name; };

static UnicodeNameTable greekTable[] = {
    { 0x03B1, "alpha" },
    { 0x03B2, "beta" },
    { 0x03B3, "gamma" },
    { 0x03B4, "delta" },
    { 0x03B5, "epsilon" },
    { 0x03F5, "varepsilon" },
    { 0x03B6, "zeta" },
    { 0x03B7, "eta" },
    { 0x03B8, "theta" },
    { 0x03D1, "vartheta" },
    { 0x03B9, "iota" },
    { 0x03BA, "kappa" },
    { 0x03F0, "varkappa" },
    { 0x03BB, "lambda" },
    { 0x03BC, "mu" },
    { 0x03BD, "nu" },
    { 0x03BE, "xi" },
    { 0x03C0, "pi" },
    { 0x03D6, "varpi" },
    { 0x03C1, "rho" },
    { 0x03F1, "varrho" },
    { 0x03C3, "sigma" },
    { 0x03C2, "varsigma" },
    { 0x03C4, "tau" },
    { 0x03C5, "upsilon" },
    { 0x03D5, "phi" },
    { 0x03C6, "varphi" },
    { 0x03C7, "chi" },
    { 0x03C8, "psi" },
    { 0x03C9, "omega" },
    { 0x0393, "Gamma" },
    { 0x0394, "Delta" },
    { 0x0398, "Theta" },
    { 0x039B, "Lambda" },
    { 0x039E, "Xi" },
    { 0x03A0, "Pi" },
    { 0x03A3, "Sigma" },
    { 0x03D2, "Upsilon" },
    { 0x03A6, "Phi" },
    { 0x03A8, "Psi" },
    { 0x03A9, "Omega" },
    { 0, 0 }
};

static UnicodeNameTable arrowTable[] = {
    { 0x2190, "leftarrow" },
    { 0x2191, "uparrow" },
    { 0x2192, "rightarrow" },
    { 0x2193, "downarrow" },
    { 0x2194, "leftrightarrow" },
    { 0x2195, "updownarrow" },
    { 0x2196, "nwarrow" },
    { 0x2197, "nearrow" },
    { 0x2198, "searrow" },
    { 0x2199, "swarrow" },
    { 0x219A, "nleftarrow" },
    { 0x219B, "nrightarrow" },
    { 0x219C, "" },
    { 0x219D, "rightsquigarrow" },
    { 0x219E, "twoheadleftarrow" },
    { 0x219F, "" },
    { 0x21A0, "twoheadrightarrow" },
    { 0x21A1, "" },
    { 0x21A2, "leftarrowtail" },
    { 0x21A3, "rightarrowtail" },
    { 0x21A4, "" },
    { 0x21A5, "" },
    { 0x21A6, "mapsto" },
    { 0x21A7, "" },
    { 0x21A8, "" },
    { 0x21A9, "hookleftarrow" },
    { 0x21AA, "hookrightarrow" },
    { 0x21AB, "looparrowleft" },
    { 0x21AC, "looparrowright" },
    { 0x21AD, "leftrightsquigarrow" },
    { 0x21AE, "nleftrightarrow" },
    { 0x21AF, "" },
    { 0x21B0, "Lsh" },
    { 0x21B1, "Rsh" },
    { 0x21B2, "" },
    { 0x21B3, "" },
    { 0x21B4, "" },
    { 0x21B5, "carriagereturn" },
    { 0x21B6, "curvearrowleft" },
    { 0x21B7, "curvearrowright" },
    { 0x21B8, "" },
    { 0x21B9, "" },
    { 0x21BA, "acwopencirclearrow" },
    { 0x21BB, "cwopencirclearrow" },
    { 0x21BC, "leftharpoonup" },
    { 0x21BD, "leftharpoondown" },
    { 0x21BE, "upharpoonright" },
    { 0x21BF, "upharpoonleft" },
    { 0x21C0, "rightharpoonup" },
    { 0x21C1, "rightharpoondown" },
    { 0x21C2, "downharpoonright" },
    { 0x21C3, "downharpoonleft" },
    { 0x21C4, "rightleftarrows" },
    { 0x21C5, "" },
    { 0x21C6, "leftrightarrows" },
    { 0x21C7, "leftleftarrows" },
    { 0x21C8, "upuparrows" },
    { 0x21C9, "rightrightarrows" },
    { 0x21CA, "downdownarrows" },
    { 0x21CB, "leftrightharpoons" },
    { 0x21CC, "rightleftharpoons" },
    { 0x21CD, "nLeftarrow" },
    { 0x21CE, "nLeftrightarrow" },
    { 0x21CF, "nRightarrow" },
    { 0x21D0, "Leftarrow" },
    { 0x21D1, "Uparrow" },
    { 0x21D2, "Rightarrow" },
    { 0x21D3, "Downarrow" },
    { 0x21D4, "Leftrightarrow" },
    { 0x21D5, "Updownarrow" },
    { 0x21D6, "" },
    { 0x21D7, "" },
    { 0x21D8, "" },
    { 0x21D9, "" },
    { 0x21DA, "Lleftarrow" },
    { 0x21DB, "Rrightarrow" },
    { 0x21DC, "" },
    { 0x21DD, "rightzigzagarrow" },
    { 0x21DE, "" },
    { 0x21DF, "" },
    { 0x21E0, "" },
    { 0x21E1, "" },
    { 0x21E2, "" },
    { 0x21E3, "" },
    { 0x21E4, "" },
    { 0x21E5, "" },
    { 0x21E6, "" },
    { 0x21E7, "" },
    { 0x21E8, "" },
    { 0x21E9, "" },
    { 0x21EA, "" },
    { 0x21EB, "" },
    { 0x21EC, "" },
    { 0x21ED, "" },
    { 0x21EE, "" },
    { 0x21EF, "" },
    { 0x21F0, "" },
    { 0x21F1, "" },
    { 0x21F2, "" },
    { 0x21F3, "" },
    { 0x21F4, "" },
    { 0x21F5, "" },
    { 0x21F6, "" },
    { 0x21F7, "" },
    { 0x21F8, "" },
    { 0x21F9, "" },
    { 0x21FA, "" },
    { 0x21FB, "" },
    { 0x21FC, "" },
    { 0x21FD, "" },
    { 0x21FE, "" },
    { 0x21FF, "" },
    { 0, 0 }
};

/*
    { 0x003A, "colon" },
    { 0x003A, "colon" },
    { 0x003C, "less" },
    { 0x003C, "less" },
    { 0x003E, "greater" },
    { 0x003E, "greater" },
    { 0x005C, "backslash" },
    { 0x005E, "textasciicircum" },
    { 0x007B, "lbrace" },
    { 0x007C, "vert" },
    { 0x007D, "rbrace" },
    { 0x007E, "textasciitilde" },
    { 0x00A1, "textexclamdown" },
    { 0x00A7, "S" },
    { 0x00AC, "neg" },
    { 0x00B0, "degree" },
    { 0x00B1, "pm" },
    { 0x00B6, "P" },
    { 0x00BF, "textquestiondown" },
    { 0x00D7, "times" },
    { 0x00D8, "O" },
    { 0x00F7, "div" },
    { 0x00F8, "o" },
    { 0x019B, "lambdabar" },
    { 0x0300, "grave" },
    { 0x0301, "acute" },
    { 0x0302, "hat" },
    { 0x0304, "bar" },
    { 0x030A, "ocirc" },
    { 0x0338, "not" },
    { 0x2013, "endash" },
    { 0x2014, "emdash" },
    { 0x2022, "bullet" },
    { 0x2026, "ldots" },
    { 0x2032, "prime" },
    { 0x2035, "backprime" },
    { 0x20D0, "leftharpoonaccent" },
    { 0x20D1, "rightharpoonaccent" },
    { 0x20D6, "overleftarrow" },
    { 0x20D7, "vec" },
    { 0x20E1, "overleftrightarrow" },
    { 0x2111, "Im" },
    { 0x2118, "wp" },
    { 0x211C, "Re" },
    { 0x2127, "mho" },
    { 0x2309, "rceil" },
    { 0x2329, "langle" },
    { 0x232A, "rangle" },
    { 0x24C8, "circledS" },
    { 0x25B3, "bigtriangleup" },
    { 0x25B4, "blacktriangle" },
    { 0x25B5, "vartriangle" },
    { 0x25B6, "blacktriangleright" },
    { 0x25BD, "bigtriangledown" },
    { 0x25BE, "blacktriangledown" },
    { 0x25C0, "blacktriangleleft" },
    { 0x25CB, "bigcirc" },
    { 0x2605, "bigstar" },
    { 0x2660, "spadesuit" },
    { 0x2661, "heartsuit" },
    { 0x2662, "diamondsuit" },
    { 0x2663, "clubsuit" },
    { 0x2666, "diamondsuit" },
    { 0x266D, "flat" },
    { 0x266E, "natural" },
    { 0x266F, "sharp" },
    { 0x2713, "checkmark" },
    { 0x2720, "maltese" },
    { 0xE201, "longleftarrow" },
    { 0xE205, "longrightarrow" },
    { 0xE29F, "gnapprox" },
    { 0xE2A0, "gneq" },
    { 0xE2A3, "lneq" },
    { 0xE2A6, "ngeqslant" },
    { 0xE2A7, "nleqslant" },
    { 0xE2A8, "nleqq" },
    { 0xE2B0, "nsupseteqq" },
    { 0xE2B2, "precnapprox" },
    { 0xE2B4, "succnapprox" },
    { 0xE2B6, "subsetneqq" },
    { 0xE2B7, "supsetneqq" },
    { 0xE2B8, "varsubsetneqq" },
    { 0xE2B9, "varsubsetneq" },
    { 0xE2BA, "varsupsetneq" },
    { 0xE2BB, "varsupsetneqq" },
    { 0xE2F4, "gtrapprox" },
    { 0xE2F5, "gtreqqless" },
    { 0xE2F6, "geqslant" },
    { 0xE2F8, "lessapprox" },
    { 0xE2F9, "lesseqqgtr" },
    { 0xE2FA, "leqslant" },
    { 0xE2FD, "precapprox" },
    { 0xE2FE, "preceq" },
    { 0xE2FF, "succapprox" },
    { 0xE300, "succeq" },
    { 0xE304, "subseteqq" },
    { 0xE305, "supseteqq" },
    { 0xE5CF, "eqslantless" },
    { 0xE5DC, "npreceq" },
    { 0xE5F1, "nsucceq" },
    { 0xE663, "Upsilon" },
*/

static UnicodeNameTable operatorTable[] = {
    { 0x2200, "forall" },
    { 0x2201, "complement" },
    { 0x2202, "partial" },
    { 0x2203, "exists" },
    { 0x2204, "nexists" },
    { 0x2205, "oslash" },
    { 0x2206, "triangle" },
    { 0x2207, "nabla" },
    { 0x2208, "in" },
    { 0x2209, "notin" },
    { 0x220A, "in" },
    { 0x220B, "ni" },
    { 0x220C, "" },
    { 0x220D, "ni" },
    { 0x220E, "blacksquare" },
    { 0x220F, "prod" },
    { 0x2210, "coprod" },
    { 0x2211, "sum" },
    { 0x2212, "minus" },
    { 0x2213, "mp" },
    { 0x2214, "dotplus" },
    { 0x2215, "slash" },
    { 0x2216, "setminus" },
    { 0x2217, "ast" },
    { 0x2218, "circ" },
    { 0x2219, "bullet" },
    { 0x221A, "surd" },
    { 0x221B, "" },
    { 0x221C, "" },
    { 0x221D, "propto" },
    { 0x221E, "infty" },
    { 0x221F, "" },
    { 0x2220, "angle" },
    { 0x2221, "measuredangle" },
    { 0x2222, "" },
    { 0x2223, "mid" },
    { 0x2224, "nmid" },
    { 0x2225, "parallel" },
    { 0x2226, "nparallel" },
    { 0x2227, "wedge" },
    { 0x2228, "vee" },
    { 0x2229, "cap" },
    { 0x222A, "cup" },
    { 0x222B, "int" },
    { 0x222C, "" },
    { 0x222D, "" },
    { 0x222E, "oint" },
    { 0x222F, "" },
    { 0x2230, "" },
    { 0x2231, "" },
    { 0x2232, "" },
    { 0x2233, "" },
    { 0x2234, "therefore" },
    { 0x2235, "because" },
    { 0x2236, "" },
    { 0x2237, "" },
    { 0x2238, "" },
    { 0x2239, "" },
    { 0x223A, "" },
    { 0x223B, "" },
    { 0x223C, "sim" },
    { 0x223D, "backsim" },
    { 0x223E, "" },
    { 0x223F, "" },
    { 0x2240, "wr" },
    { 0x2241, "" },
    { 0x2242, "eqsim" },
    { 0x2243, "simeq" },
    { 0x2244, "nsime" },
    { 0x2245, "cong" },
    { 0x2246, "" },
    { 0x2247, "ncong" },
    { 0x2248, "approx" },
    { 0x2249, "" },
    { 0x224A, "approxeq" },
    { 0x224B, "" },
    { 0x224C, "" },
    { 0x224D, "asymp" },
    { 0x224E, "Bumpeq" },
    { 0x224F, "bumpeq" },
    { 0x2250, "doteq" },
    { 0x2251, "Doteq" },
    { 0x2252, "fallingdotseq" },
    { 0x2253, "risingdotseq" },
    { 0x2254, "" },
    { 0x2255, "" },
    { 0x2256, "eqcirc" },
    { 0x2257, "circeq" },
    { 0x2258, "" },
    { 0x2259, "wedgeq" },
    { 0x225A, "" },
    { 0x225B, "" },
    { 0x225C, "triangleq" },
    { 0x225D, "" },
    { 0x225E, "" },
    { 0x225F, "" },
    { 0x2260, "neq" },
    { 0x2261, "equiv" },
    { 0x2262, "" },
    { 0x2263, "" },
    { 0x2264, "leq" },
    { 0x2265, "geq" },
    { 0x2266, "leqq" },
    { 0x2267, "geqq" },
    { 0x2268, "" },
    { 0x2269, "" },
    { 0x226A, "ll" },
    { 0x226B, "gg" },
    { 0x226C, "between" },
    { 0x226D, "" },
    { 0x226E, "nless" },
    { 0x226F, "ngtr" },
    { 0x2270, "nleq" },
    { 0x2271, "ngeq" },
    { 0x2272, "lesssim" },
    { 0x2273, "gtrsim" },
    { 0x2274, "" },
    { 0x2275, "" },
    { 0x2276, "lessgtr" },
    { 0x2277, "gtrless" },
    { 0x2278, "" },
    { 0x2279, "" },
    { 0x227A, "prec" },
    { 0x227B, "succ" },
    { 0x227C, "preccurlyeq" },
    { 0x227D, "succcurlyeq" },
    { 0x227E, "precsim" },
    { 0x227F, "succsim" },
    { 0x2280, "nprec" },
    { 0x2281, "nsucc" },
    { 0x2282, "subset" },
    { 0x2283, "supset" },
    { 0x2284, "nsubset" },
    { 0x2286, "subseteq" },
    { 0x2287, "supseteq" },
    { 0x2288, "nsubseteq" },
    { 0x2289, "nsupseteq" },
    { 0x228A, "subsetneq" },
    { 0x228B, "supsetneq" },
    { 0x228C, "" },
    { 0x228D, "" },
    { 0x228E, "uplus" },
    { 0x228F, "sqsubset" },
    { 0x2290, "sqsupset" },
    { 0x2291, "sqsubseteq" },
    { 0x2292, "sqsupseteq" },
    { 0x2293, "sqcap" },
    { 0x2294, "sqcup" },
    { 0x2295, "oplus" },
    { 0x2296, "ominus" },
    { 0x2297, "otimes" },
    { 0x2298, "oslash" },
    { 0x2299, "odot" },
    { 0x229A, "circledcirc" },
    { 0x229B, "circledast" },
    { 0x229C, "" },
    { 0x229D, "circleddash" },
    { 0x229E, "boxplus" },
    { 0x229F, "boxminus" },
    { 0x22A0, "boxtimes" },
    { 0x22A1, "boxdot" },
    { 0x22A2, "" },
    { 0x22A3, "dashv" },
    { 0x22A4, "top" },
    { 0x22A5, "" },
    { 0x22A6, "" },
    { 0x22A7, "" },
    { 0x22A8, "vDash" },
    { 0x22A9, "Vdash" },
    { 0x22AA, "Vvdash" },
    { 0x22AB, "" },
    { 0x22AC, "nvdash" },
    { 0x22AD, "nvDash" },
    { 0x22AE, "nVdash" },
    { 0x22AF, "nVDash" },
    { 0x22B1, "" },
    { 0x22B2, "vartriangleleft" },
    { 0x22B3, "vartriangleright" },
    { 0x22B4, "trianglelefteq" },
    { 0x22B5, "trianglerighteq" },
    { 0x22B6, "" },
    { 0x22B7, "" },
    { 0x22B8, "multimap" },
    { 0x22B9, "" },
    { 0x22BA, "intercal" },
    { 0x22BB, "veebar" },
    { 0x22BC, "barwedge" },
    { 0x22BD, "" },
    { 0x22BE, "" },
    { 0x22BF, "" },
    { 0x22C1, "" },
    { 0x22C2, "" },
    { 0x22C3, "" },
    { 0x22C4, "diamond" },
    { 0x22C5, "cdot" },
    { 0x22C6, "star" },
    { 0x22C7, "divideontimes" },
    { 0x22C8, "" },
    { 0x22C9, "ltimes" },
    { 0x22CA, "rtimes" },
    { 0x22CB, "leftthreetimes" },
    { 0x22CC, "rightthreetimes" },
    { 0x22CD, "backsimeq" },
    { 0x22CE, "curlyvee" },
    { 0x22CF, "curlywedge" },
    { 0x22D0, "Subset" },
    { 0x22D1, "Supset" },
    { 0x22D2, "Cap" },
    { 0x22D3, "Cup" },
    { 0x22D4, "pitchfork" },
    { 0x22D5, "" },
    { 0x22D6, "lessdot" },
    { 0x22D7, "gtrdot" },
    { 0x22D8, "lll" },
    { 0x22D9, "ggg" },
    { 0x22DA, "lesseqgtr" },
    { 0x22DB, "gtreqless" },
    { 0x22DC, "eqless" },
    { 0x22DD, "eqgtr" },
    { 0x22DE, "curlyeqprec" },
    { 0x22DF, "curlyeqsucc" },
    { 0x22E0, "" },
    { 0x22E1, "" },
    { 0x22E2, "" },
    { 0x22E3, "" },
    { 0x22E4, "" },
    { 0x22E5, "" },
    { 0x22E6, "lnsim" },
    { 0x22E7, "gnsim" },
    { 0x22E8, "precnsim" },
    { 0x22E9, "succnsim" },
    { 0x22EA, "ntriangleleft" },
    { 0x22EB, "ntriangleright" },
    { 0x22EC, "ntrianglelefteq" },
    { 0x22ED, "ntrianglerighteq" },
    { 0x22EE, "vdots" },
    { 0x22EF, "cdots" },
    { 0x22F0, "adots" },
    { 0x22F1, "ddots" },
    { 0x22F2, "" },
    { 0x22F3, "" },
    { 0x22F4, "" },
    { 0x22F5, "" },
    { 0x22F6, "" },
    { 0x22F7, "" },
    { 0x22F8, "" },
    { 0x22F9, "" },
    { 0x22FA, "" },
    { 0x22FB, "" },
    { 0x22FC, "" },
    { 0x22FD, "" },
    { 0x22FE, "" },
    { 0x22FF, "" },
    { 0, 0 }
};

KFORMULA_NAMESPACE_END
