Point(1) = {0.2,0.6,0,0.1};
Point(2) = {1.2,0.6,0,0.1};
Point(3) = {1.2,0.1,0,0.1};
Point(4) = {0.3,0.1,0,0.1};
Point(5) = {0.7,0,0,0.1};
CatmullRom(1) = {3,2,1,4,5,3};
Line Loop(2) = {1};
Plane Surface(3) = {2};
Extrude {0,0,1} {
  Surface{3};
}
