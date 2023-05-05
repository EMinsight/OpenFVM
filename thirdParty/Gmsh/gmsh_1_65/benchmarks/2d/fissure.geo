eps = 1.e-3 ;

Point(1) = {-1,-1,0,.1} ;
Point(2) = {-1,1,0,.1} ;
Point(3) = {1,-1,0,.1} ;
Point(4) = {1,1,0,.1} ;
Point(5) = {-1,0,0,.06} ;
Point(6) = {0,0,0,.03} ;
Point(7) = {-1,0+eps,0,.06} ;
Line(1) = {5,1};
Line(2) = {1,3};
Line(3) = {3,4};
Line(4) = {4,2};
Line(5) = {2,7};
Line(6) = {7,6};
Line(7) = {6,5};
Line Loop(8) = {5,6,7,1,2,3,4};
Plane Surface(9) = {8};