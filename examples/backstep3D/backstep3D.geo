Mesh.MshFileVersion=1;

h = 0.04;

y0 = 5*h; 

z = 7.5*h;

l1 = 10*h;
l2 = 40*h;

n = 10;
r = 5;

Point(1) = {-l1,h,0,0.1};
Point(2) = {0,h,0,0.1};
Point(3) = {0,0,0,0.1};
Point(4) = {l2,0,0,0.1};
Point(5) = {l2,h,0,0.1};
Point(6) = {l2,y0+h,0,0.1};
Point(7) = {0,y0+h,0,0.1};
Point(8) = {-l1,y0+h,0,0.1};

Line(1) = {8,7};
Line(2) = {6,7};
Line(3) = {6,5};
Line(4) = {5,2};
Line(5) = {7,2};
Line(6) = {8,1};
Line(7) = {1,2};
Line(8) = {3,2};
Line(9) = {4,3};
Line(10) = {4,5};

Line Loop(11) = {1,5,-7,-6};
Plane Surface(12) = {11};
Line Loop(13) = {2,5,-4,-3};
Plane Surface(14) = {13};
Line Loop(15) = {4,-8,-9,10};
Plane Surface(16) = {15};

Transfinite Line {1,7} = l1/h*n+1 Using Progression 0.96;
Transfinite Line {2,4,9} = l2/h*n+1 Using Progression 0.99;
Transfinite Line {6,5,3} = y0/h*n+1 Using Progression 0.92;
Transfinite Line {8,10} = r*n+1 Using Progression 1.0;

Transfinite Surface {14} = {7,6,5,2};
Transfinite Surface {16} = {2,5,4,3};
Transfinite Surface {12} = {7,2,1,8};
Recombine Surface {14,16,12};

Extrude Surface {12, {0,0,z} } {
  Layers { {z/h*n}, {9000}, {1.0} };
  Recombine;
};

Extrude Surface {14, {0,0,z} } {
  Layers { {z/h*n}, {9000}, {1.0} };
  Recombine;
};

Extrude Surface {16, {0,0,z} } {
  Layers { {z/h*n}, {9000}, {1.0} };
  Recombine;
};

// walls - slip
Physical Surface(82) = {25,47};
// walls - no slip
Physical Surface(81) = {82,60,16,14,38,12};
Physical Surface(83) = {77,73,33};
// inlet
Physical Surface(84) = {37};
// outlet
Physical Surface(85) = {59,81};

Physical Volume(86) = {9000};
