function mesh = load_gmsh(filename)
% Reads a mesh in msh format, version 1
mesh = [];    
mesh.MIN = zeros(3, 1);
mesh.MAX = zeros(3, 1);
fid = fopen(filename, 'r');
while 1
  endoffile = 0;
  while 1
    tline = fgetl(fid);
    if feof(fid), endoffile=1, break, end
    if tline(1) == '$', break, end  
  end
  if endoffile == 1, break, end
  if tline(2) == 'N' && tline(3) == 'O'
    disp('reading nodes')
    mesh.nbNod = fscanf(fid, '%d', 1);
    mesh.POS = zeros(mesh.nbNod, 3);
    for(I=1:mesh.nbNod)
      iNod = fscanf(fid, '%d', 1);
      X = fscanf(fid, '%g', 3);
      IDS(iNod) = I;
      if (I == 1)
	mesh.MIN = X;
	mesh.MAX = X;
      else
	if(mesh.MAX(1) < X(1)) mesh.MAX(1) = X(1); end
	if(mesh.MAX(2) < X(2)) mesh.MAX(2) = X(2); end
	if(mesh.MAX(3) < X(3)) mesh.MAX(3) = X(3); end
	if(mesh.MIN(1) > X(1)) mesh.MIN(1) = X(1); end
	if(mesh.MIN(2) > X(2)) mesh.MIN(2) = X(2); end
	if(mesh.MIN(3) > X(3)) mesh.MIN(3) = X(3); end
      end
      mesh.POS(I, 1) = X(1);
      mesh.POS(I, 2) = X(2);
      mesh.POS(I, 3) = X(3);
    end
    tline = fgetl(fid);
    disp('nodes have been read')
  elseif tline(2) == 'E' && tline(3) == 'L'
    disp('reading elements')
    mesh.nbElm = fscanf(fid, '%d', 1);
    mesh.ELE_INFOS = zeros(mesh.nbElm, 5);
    mesh.nbPoints = 0;
    mesh.nbLines = 0;
    mesh.nbTriangles = 0;
    mesh.nbQuads = 0;
    mesh.nbTets = 0;
    mesh.nbHexas = 0;
    mesh.nbPrisms = 0;
    mesh.nbPyramids = 0;
    % comment next 8 lines to get "tight" arrays (will slow down reading)
    mesh.POINTS = zeros(mesh.nbElm, 2);
    mesh.LINES = zeros(mesh.nbElm, 3);
    mesh.TRIANGLES = zeros(mesh.nbElm, 4);
    mesh.QUADS = zeros(mesh.nbElm, 5);
    mesh.TETS = zeros(mesh.nbElm, 5);
    mesh.HEXAS = zeros(mesh.nbElm, 9);
    mesh.PRISMS = zeros(mesh.nbElm, 7);
    mesh.PYRAMIDS = zeros(mesh.nbElm, 6);
    for(I = 1:mesh.nbElm)
      mesh.ELE_INFOS(I, :) = fscanf(fid, '%d', 5);
      NODES_ELEM = fscanf(fid, '%d', mesh.ELE_INFOS(I, 5));
      if(mesh.ELE_INFOS(I, 2) == 15) %% point
	mesh.nbPoints = mesh.nbPoints + 1;
	mesh.POINTS(mesh.nbPoints, 1) = IDS(NODES_ELEM(1));
	mesh.POINTS(mesh.nbPoints, 2) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 1) %% line
	mesh.nbLines = mesh.nbLines + 1;
	mesh.LINES(mesh.nbLines, 1) = IDS(NODES_ELEM(1));
	mesh.LINES(mesh.nbLines, 2) = IDS(NODES_ELEM(2));   
	mesh.LINES(mesh.nbLines, 3) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 2) %% triangle
	mesh.nbTriangles = mesh.nbTriangles + 1;
	mesh.TRIANGLES(mesh.nbTriangles, 1) = IDS(NODES_ELEM(1));
	mesh.TRIANGLES(mesh.nbTriangles, 2) = IDS(NODES_ELEM(2));
	mesh.TRIANGLES(mesh.nbTriangles, 3) = IDS(NODES_ELEM(3));
	mesh.TRIANGLES(mesh.nbTriangles, 4) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 3) %% quadrangle
	mesh.nbQuads = mesh.nbQuads + 1;
	mesh.QUADS(mesh.nbQuads, 1) = IDS(NODES_ELEM(1));
	mesh.QUADS(mesh.nbQuads, 2) = IDS(NODES_ELEM(2));
	mesh.QUADS(mesh.nbQuads, 3) = IDS(NODES_ELEM(3));
	mesh.QUADS(mesh.nbQuads, 4) = IDS(NODES_ELEM(4));
	mesh.QUADS(mesh.nbQuads, 5) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 4) %% tetrahedron
	mesh.nbTets = mesh.nbTets + 1;
	mesh.TETS(mesh.nbTets, 1) = IDS(NODES_ELEM(1));
	mesh.TETS(mesh.nbTets, 2) = IDS(NODES_ELEM(2));
	mesh.TETS(mesh.nbTets, 3) = IDS(NODES_ELEM(3));
	mesh.TETS(mesh.nbTets, 4) = IDS(NODES_ELEM(4));
	mesh.TETS(mesh.nbTets, 5) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 5) %% hexahedron
	mesh.nbHexas = mesh.nbHexas + 1;
	mesh.HEXAS(mesh.nbHexas, 1) = IDS(NODES_ELEM(1));
	mesh.HEXAS(mesh.nbHexas, 2) = IDS(NODES_ELEM(2));
	mesh.HEXAS(mesh.nbHexas, 3) = IDS(NODES_ELEM(3));
	mesh.HEXAS(mesh.nbHexas, 4) = IDS(NODES_ELEM(4));
	mesh.HEXAS(mesh.nbHexas, 5) = IDS(NODES_ELEM(5));
	mesh.HEXAS(mesh.nbHexas, 6) = IDS(NODES_ELEM(6));
	mesh.HEXAS(mesh.nbHexas, 7) = IDS(NODES_ELEM(7));
	mesh.HEXAS(mesh.nbHexas, 8) = IDS(NODES_ELEM(8));
	mesh.HEXAS(mesh.nbHexas, 9) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 6) %% prism
	mesh.nbPrisms = mesh.nbPrisms + 1;
	mesh.PRISMS(mesh.nbPrisms, 1) = IDS(NODES_ELEM(1));
	mesh.PRISMS(mesh.nbPrisms, 2) = IDS(NODES_ELEM(2));
	mesh.PRISMS(mesh.nbPrisms, 3) = IDS(NODES_ELEM(3));
	mesh.PRISMS(mesh.nbPrisms, 4) = IDS(NODES_ELEM(4));
	mesh.PRISMS(mesh.nbPrisms, 5) = IDS(NODES_ELEM(5));
	mesh.PRISMS(mesh.nbPrisms, 6) = IDS(NODES_ELEM(6));
	mesh.PRISMS(mesh.nbPrisms, 7) = mesh.ELE_INFOS(I, 3);
      end
      if(mesh.ELE_INFOS(I, 2) == 7) %% pyramid
	mesh.nbPyramids = mesh.nbPyramids + 1;
	mesh.PYRAMIDS(mesh.nbPyramids, 1) = IDS(NODES_ELEM(1));
	mesh.PYRAMIDS(mesh.nbPyramids, 2) = IDS(NODES_ELEM(2));
	mesh.PYRAMIDS(mesh.nbPyramids, 3) = IDS(NODES_ELEM(3));
	mesh.PYRAMIDS(mesh.nbPyramids, 4) = IDS(NODES_ELEM(4));
	mesh.PYRAMIDS(mesh.nbPyramids, 5) = IDS(NODES_ELEM(5));
	mesh.PYRAMIDS(mesh.nbPyramids, 6) = IDS(NODES_ELEM(6));
	mesh.PYRAMIDS(mesh.nbPyramids, 7) = mesh.ELE_INFOS(I, 3);
      end
    end
    tline = fgetl(fid);
    disp('elements have been read')
  end
end
fclose(fid);
