Mesh.MshFileVersion=1;
nT=25;
nC=15;
nt=8;
T=1;
lc=0.1;
lc2=0.1;

Point(1) = {0.0,0.0,0.0,lc};
Point(2) = {0.0,1,0.0,lc};
Point(3) = {1,0,0.0,lc2};
Point(4) = {-1,0,0.0,lc};
Point(5) = {0,-1,0.0,lc};

Line(1) = {2,3};
Line(2) = {3,5};
Line(3) = {5,4};
Line(4) = {4,2};
Line Loop(5) = {1,2,3,4};
Plane Surface(6) = {5};

Transfinite Line {2,3,4,1} = 8 Using Progression 1.0;
Transfinite Surface {6} = {2,3,5,4};
Recombine Surface {6};

Extrude Surface {6, {0.0,0.0,5.0}}
  { Recombine; Layers { {nT}, {9000}, {T} }; };

Extrude Surface {6, {1.0,1.0,0.0}, {2.0,0.0,0.0}, -Pi/2}
  { Recombine; Layers { {nC}, {9001}, {T} }; };

Extrude Surface {50, {1.0,-1.0,0.0}}
  { Recombine; Layers { {nt}, {9002}, {T} }; };


// Inlet
Physical Surface(73) = {28};

// Outlet
Physical Surface(74) = {72};
//Physical Surface(74) = {6};
//Physical Surface(74) = {50};

// Walls
Physical Surface(75) = {15,27,23,19,37,45,49,41,59,67,71,63};

Physical Volume(76) = {9000};
Physical Volume(77) = {9001};
Physical Volume(78) = {9002};
