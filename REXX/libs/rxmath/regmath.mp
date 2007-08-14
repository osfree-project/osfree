%
% Manual for RegMath
% Copyright 2001, Patrick TJ McPhee
% Distributed under the terms of the Mozilla Public Licence
% You can obtain a copy of the licence at http://www.mozilla.org/MPL
% The Original Code is regmath
% The Initial Developer is Patrick TJ McPhee
% $Header: C:/ptjm/rexx/math/tmp/RCS/regmath.mp 1.1 2001/11/04 19:01:12 ptjm Exp $
%

%% rexx math diagrams

prologues := 2;

string symfont,rmfont,itfont;
symfont := "rpsyr";
rmfont := "rptmr";
itfont := "rptmri";
defaultfont := itfont;

% regmath.1 should be converted to rat.pdf for inclusion in the document
beginfig(1);
  top lft z1 = (0,1in) = bot lft z2 + z1 = bot rt z3 - (3in,-1in);
  z4 = .2[z1,z2];
  z10 = z3 - z1;
  z5 = z1 + (unitvector z10) * abs (z4 - z1);
  lft z6 = (0, x5);
  bot z8 = (x5,0);
  x7 = x8; y6=y7;

  pickup pencircle xscaled 1pt yscaled 3pt;
  path p;
  p = z4{right}..{-y10,x10}z5;
  draw p withcolor .5 white;
  draw z1--z2--z3--cycle;
  draw z6--z7--z8;

  % theta
  label.lrt(char(113) infont symfont, point .5 of p);

  % label the sides
  label.lft("x", .5[z1,z2]);
  label.bot("y", .5[z2,z3]);
  label.urt("r", .5[z1,z3]);

  % draw the formulae
  pair o, oo;
  picture p, th, eq;
  string rm[], it[];

  rm1="sine ";
  it1="y / r";
  rm2="cosine ";
  it2="x / r";
  rm3="tangent ";
  it3="y / x";

  rm4="cosecant ";
  it4="r / y";
  rm5="secant ";
  it5="r / x";
  rm6="cotangent ";
  it6="x / y";

  o = (3.1in,1in);
  th = char(113) infont symfont;
  eq = " = " infont rmfont;

  for i := 1 upto 6:
    o := o - (0, 12pt);
    oo := o;
    p := rm[i] infont rmfont;
    draw p shifted oo;
    oo := oo + (xpart lrcorner p, 0);
    draw th shifted oo;
    oo := oo + (xpart lrcorner th, 0);
    draw eq shifted oo;
    oo := oo + (xpart lrcorner eq, 0);
    p := it[i] infont itfont;
    draw p shifted oo;
  endfor;    

  currentpicture := currentpicture shifted -llcorner currentpicture;

endfig;

% regmath.2 should be converted to sincos.pdf for inclusion in the document
% these are sine and cosine plotted against t from 0 to 3pi
beginfig(2);
  path p;

  % divide pi into 8 and note the value of sine and cosine for each. This
  % will print a quadratic approximation, but how far off can it be?
  % (these values were generated using rexxmath)
  un = .5in;
  z1 = (0un, 0.000000un);
  z2 = (0.3926990816987242un, 0.382683un);
  z3 = (0.7853981633974484un, 0.707107un);
  z4 = (1.178097245096173un, 0.923880un);
  z5 = (1.570796326794897un, 1.000000un);
  z6 = (1.963495408493621un, 0.923880un);
  z7 = (2.356194490192345un, 0.707107un);
  z8 = (2.748893571891069un, 0.382683un);
  z9 = (3.141592653589793un, 0.000000un);
  z10 = (3.534291735288517un, -0.382683un);
  z11 = (3.926990816987241un, -0.707107un);
  z12 = (4.319689898685965un, -0.923880un);
  z13 = (4.712388980384689un, -1.000000un);
  z14 = (5.105088062083413un, -0.923880un);
  z15 = (5.497787143782137un, -0.707107un);
  z16 = (5.890486225480861un, -0.382683un);
  z17 = (6.283185307179585un, -0.000000un);
  z18 = (6.675884388878309un, 0.382683un);
  z19 = (7.068583470577033un, 0.707107un);
  z20 = (7.461282552275757un, 0.923880un);
  z21 = (7.853981633974481un, 1.000000un);
  z22 = (8.246680715673205un, 0.923880un);
  z23 = (8.639379797371929un, 0.707107un);
  z24 = (9.032078879070653un, 0.382683un);
  z25 = (9.424777960769377un, 0.000000un);

  % axes
  pickup pencircle xscaled 2 pt yscaled 2 pt;
  drawarrow (0,0)--(x25+.5un,0);
  drawarrow (0,0)--(0,1.5un);
  drawarrow (0,0)--(0,-1.5un);
  
  label.bot("t" infont itfont, (x25+.5un,0));

  picture lab[];

  lab[5] = "p/2" infont symfont;
  lab[9] = "p" infont symfont;
  lab[13] = "3p/2" infont symfont;
  lab[17] = "2p" infont symfont;
  lab[21] = "5p/2" infont symfont;
  lab[25] = "3p" infont symfont;

  pickup pencircle xscaled 1 pt yscaled 1.2 pt;
  for i = 5,9,13,17,21,25:
    draw (x[i],0)--(x[i],4pt);
    label.bot(lab[i], (x[i],0));
  endfor;

  draw (0,.5un)--(4pt,.5un);
  draw (0,-.5un)--(4pt,-.5un);
  draw (0,un)--(4pt,un);
  draw (0,-un)--(4pt,-un);

  label.lft("0.5" infont symfont, (0,.5un));
  label.lft("-0.5" infont symfont, (0,-.5un));
  label.lft("1.0" infont symfont, (0,1un));
  label.lft("-1.0" infont symfont, (0,-un));

  draw z1 for i = 2 upto 25: ..z[i] endfor withcolor red;
  label.rt("sin(t)", z22);


  numeric x[], y[];

  z1 = (0un, 1.000000un);
  z2 = (0.3926990816987242un, 0.923880un);
  z3 = (0.7853981633974484un, 0.707107un);
  z4 = (1.178097245096173un, 0.382683un);
  z5 = (1.570796326794897un, -0.000000un);
  z6 = (1.963495408493621un, -0.382683un);
  z7 = (2.356194490192345un, -0.707107un);
  z8 = (2.748893571891069un, -0.923880un);
  z9 = (3.141592653589793un, -1.000000un);
  z10 = (3.534291735288517un, -0.923880un);
  z11 = (3.926990816987241un, -0.707107un);
  z12 = (4.319689898685965un, -0.382683un);
  z13 = (4.712388980384689un, -0.000000un);
  z14 = (5.105088062083413un, 0.382683un);
  z15 = (5.497787143782137un, 0.707107un);
  z16 = (5.890486225480861un, 0.923880un);
  z17 = (6.283185307179585un, 1.000000un);
  z18 = (6.675884388878309un, 0.923880un);
  z19 = (7.068583470577033un, 0.707107un);
  z20 = (7.461282552275757un, 0.382683un);
  z21 = (7.853981633974481un, 0.000000un);
  z22 = (8.246680715673205un, -0.382683un);
  z23 = (8.639379797371929un, -0.707107un);
  z24 = (9.032078879070653un, -0.923880un);
  z25 = (9.424777960769377un, -1.000000un);

  draw z1 for i = 2 upto 25: ..z[i] endfor withcolor blue;
  label.rt("cos(t)", z25);

  currentpicture := currentpicture shifted -llcorner currentpicture;

endfig;

% regmath.3 should be converted to exp.pdf for inclusion in the document
beginfig(3);
  pickup pencircle xscaled 2 pt yscaled 2 pt;
  draw (-2.1un,0)--(4.3un, 0);
  draw (0,-2.1un)--(0, 4un);

  pickup pencircle xscaled 1 pt yscaled 1.2 pt;
  for i = -2,-1:
    draw(i*un,0)--(i*un,4pt);
    draw(0,i*un)--(4pt,i*un);
    label.bot(decimal(i) infont symfont, (i*un,0));
    label.lft(decimal(i) infont symfont, (0,i*un));
  endfor;
  for i = 1 upto 4:
    draw(i*un,0)--(i*un,4pt);
    draw(0,i*un)--(4pt,i*un);
    label.bot(decimal(i) infont symfont, (i*un,0));
    label.lft(decimal(i) infont symfont, (0,i*un));
  endfor;


  z5 = (-1.2un, -1.509461un);
  z6 = (-1.0un, -1.175201un);
  z7 = (-0.8un, -0.888106un);
  z8 = (-0.6un, -0.636654un);
  z9 = (-0.4un, -0.410752un);
  z10 = (-0.2un, -0.201336un);
  z11 = (0un, 0.000000un);
  z12 = (0.2un, 0.201336un);
  z13 = (0.4un, 0.410752un);
  z14 = (0.6un, 0.636654un);
  z15 = (0.8un, 0.888106un);
  z16 = (1.0un, 1.175201un);
  z17 = (1.2un, 1.509461un);
  z18 = (1.4un, 1.904302un);
  z19 = (1.6un, 2.375568un);
  z20 = (1.8un, 2.942174un);
  z21 = (2.0un, 3.626860un);


  draw z5 for i = 6 upto 21 : ..z[i] endfor withcolor red;
  label.lrt("sinh x", z18);

  numeric x[], y[];
  z1 = (-2un, 3.762196un);
  z2 = (-1.8un, 3.107473un);
  z3 = (-1.6un, 2.577464un);
  z4 = (-1.4un, 2.150898un);
  z5 = (-1.2un, 1.810656un);
  z6 = (-1.0un, 1.543081un);
  z7 = (-0.8un, 1.337435un);
  z8 = (-0.6un, 1.185465un);
  z9 = (-0.4un, 1.081072un);
  z10 = (-0.2un, 1.020067un);
  z11 = (0un, 1.000000un);
  z12 = (0.2un, 1.020067un);
  z13 = (0.4un, 1.081072un);
  z14 = (0.6un, 1.185465un);
  z15 = (0.8un, 1.337435un);
  z16 = (1.0un, 1.543081un);
  z17 = (1.2un, 1.810656un);
  z18 = (1.4un, 2.150898un);
  z19 = (1.6un, 2.577464un);
  z20 = (1.8un, 3.107473un);
  z21 = (2.0un, 3.762196un);
  draw z1 for i = 1 upto 21 : ..z[i] endfor withcolor blue;
  label.urt("cosh x", z3);

  numeric x[], y[];
  z1 = (-2un, 0.135335un);
  z2 = (-1.8un, 0.165299un);
  z3 = (-1.6un, 0.201897un);
  z4 = (-1.4un, 0.246597un);
  z5 = (-1.2un, 0.301194un);
  z6 = (-1.0un, 0.367879un);
  z7 = (-0.8un, 0.449329un);
  z8 = (-0.6un, 0.548812un);
  z9 = (-0.4un, 0.670320un);
  z10 = (-0.2un, 0.818731un);
  z11 = (0un, 1.000000un);
  z12 = (0.2un, 1.221403un);
  z13 = (0.4un, 1.491825un);
  z14 = (0.6un, 1.822119un);
  z15 = (0.8un, 2.225541un);
  z16 = (1.0un, 2.718282un);
  z17 = (1.2un, 3.320117un);
  z18 = (1.4un, 4.055200un);
  draw z1 for i = 1 upto 18 : ..z[i] endfor withcolor green;
  label.ulft("exp x", z16);

  numeric x[], y[];
  z1 = (0.2un, -1.609438un);
  z2 = (0.4un, -0.916291un);
  z3 = (0.6un, -0.510826un);
  z4 = (0.8un, -0.223144un);
  z5 = (1.0un, 0.000000un);
  z6 = (1.2un, 0.182322un);
  z7 = (1.4un, 0.336472un);
  z8 = (1.6un, 0.470004un);
  z9 = (1.8un, 0.587787un);
  z10 = (2.0un, 0.693147un);
  z11 = (2.2un, 0.788457un);
  z12 = (2.4un, 0.875469un);
  z13 = (2.6un, 0.955511un);
  z14 = (2.8un, 1.029619un);
  z15 = (3.0un, 1.098612un);
  z16 = (3.2un, 1.163151un);
  z17 = (3.4un, 1.223775un);
  z18 = (3.6un, 1.280934un);
  z19 = (3.8un, 1.335001un);
  z20 = (4.0un, 1.386294un);
  z21 = (4.2un, 1.435085un);
  draw z1 for i = 1 upto 21 : ..z[i] endfor withcolor red+blue;
  label.rt("log x", z2);

  currentpicture := currentpicture shifted -llcorner currentpicture;
endfig;
end
