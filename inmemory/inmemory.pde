int LeftMargin = 40;
int TopMargin = 40;
int Spacing = 100;
int PointSize = 12;
int SelectX = 0;
int SelectY = 0;
boolean DrawMesh = false;

// TODO: show connections (mesh)
class Block {
  int level = 0;
  int numPointsX = 2;
  int numPointsY = 2;
  int coordX = 0;
  int coordY = 0;
  float startX = 0;
  float startY = 0;
  float spacingX = Spacing;
  float spacingY = Spacing;
  
  void DrawMesh() {    
    if (coordX == SelectX && coordY == SelectY) {
      stroke(0, 36, 181, 100);
      /* inner */
      for (int i = 0; i < numPointsX; ++i) {
        line(startX + i * spacingX, startY, startX + i * spacingX, startY + (numPointsY - 1) * spacingY);
      }
      for (int j = 0; j < numPointsY; ++j) {
        line(startX, startY + j * spacingY, startX + (numPointsX - 1) * spacingX, startY + j * spacingY);
      }
    }
  }
  
  void Draw1() {
    //noStroke();
    fill(0, 36, 181);
    for (int i = 0; i < numPointsX; ++i) {
      for (int j = 0; j < numPointsY; ++j) {
        ellipse(startX + i * spacingX, startY + j * spacingY, PointSize, PointSize);
      }
    }
  }
  
  void Draw2() {        
    // draw the interpolated points
    fill(181, 36, 0);
    // left
    if (coordX > 0) {       //<>//
      int neighborX = coordX - 1;
      int neighborY = coordY;
      Block neighbor = Blocks[neighborY * 4 + neighborX];
      if (neighbor.numPointsY < numPointsY) { // only put imaginary points if I am more refined than my neighbor
        int numPointsBoundary = max(neighbor.numPointsY, numPointsY);
        float spacingBoundaryY = min(neighbor.spacingY, spacingY);
        float boundaryX = neighbor.startX + neighbor.spacingX * (neighbor.numPointsX - 1);
        int j = 1;
        for (; startY + j * spacingBoundaryY <= neighbor.startY + (neighbor.numPointsY - 1) * neighbor.spacingY; ++j) {
          ellipse(boundaryX, startY + j * spacingBoundaryY, PointSize, PointSize);
          if (DrawMesh && coordX == SelectX && coordY == SelectY) {
            line(boundaryX, startY + j * spacingBoundaryY, startX, startY + j * spacingBoundaryY);
            line(boundaryX, startY + j * spacingBoundaryY, boundaryX, startY + (j - 1) * spacingBoundaryY);
          }
        }
        if (DrawMesh && coordX == SelectX && coordY == SelectY) {
          line(boundaryX, startY, startX, startY);
        }
        /* check the down neighbor of the neighbor */
        boolean meFirst = true;
        if (neighborY < 3) {
          int otherNeighborX = neighborX;
          int otherNeighborY = neighborY + 1;
          Block otherNeighbor = Blocks[otherNeighborY * 4 + otherNeighborX];
          int ratio1 = numPointsY / neighbor.numPointsY;
          int ratio2 = otherNeighbor.numPointsX / neighbor.numPointsX;          
          if ((ratio1 < ratio2) || (ratio1 == ratio2 && (coordX + coordY * 4) > (otherNeighborX + otherNeighborY * 4))) {
            boundaryX = otherNeighbor.startX + (otherNeighbor.numPointsX - 1) * otherNeighbor.spacingX;
            meFirst = false;
          }
        }
        if (meFirst) {
          for (; j < numPointsBoundary; ++j) {
            ellipse(boundaryX, startY + j * spacingBoundaryY, PointSize, PointSize);
            if (DrawMesh && coordX == SelectX && coordY == SelectY) {
              line(boundaryX, startY + j * spacingBoundaryY, startX, startY + j * spacingBoundaryY);
              line(boundaryX, startY + j * spacingBoundaryY, boundaryX, startY + (j - 1) * spacingBoundaryY);
            }            
          }
        } else {
          for (; j < numPointsBoundary; ++j) {
            ellipse(boundaryX, startY + j * spacingBoundaryY, PointSize, PointSize);
            if (DrawMesh && coordX == SelectX && coordY == SelectY) {
              line(boundaryX, startY + j * spacingBoundaryY, startX, startY + j * spacingBoundaryY);
              line(boundaryX, startY + j * spacingBoundaryY, boundaryX, startY + (j - 1) * spacingBoundaryY);
            }            
          }
        }
      }
    }
    // right
    if (coordX < 3) {
      int neighborX = coordX + 1;
      int neighborY = coordY;
      Block neighbor = Blocks[neighborY * 4 + neighborX];
      if (neighbor.numPointsY < numPointsY) {
        int numPointsBoundary = max(neighbor.numPointsY, numPointsY);
        float spacingBoundaryY = min(neighbor.spacingY, spacingY);
        float boundaryX = neighbor.startX;
        for (int j = 1; j < numPointsBoundary; ++j) {
          ellipse(boundaryX, startY + j * spacingBoundaryY, PointSize, PointSize);
          if (DrawMesh && coordX == SelectX && coordY == SelectY) {
            line(boundaryX, startY + j * spacingBoundaryY, startX + (numPointsX - 1) * spacingX, startY + j * spacingBoundaryY);
            line(boundaryX, startY + j * spacingBoundaryY, boundaryX, startY + (j - 1) * spacingBoundaryY);
          }          
        }
        if (DrawMesh && coordX == SelectX && coordY == SelectY) {
          line(boundaryX, startY, startX + (numPointsX - 1) * spacingX, startY);
        }
      }
    }
    // up
    if (coordY > 0) {
      int neighborX = coordX;
      int neighborY = coordY - 1;
      Block neighbor = Blocks[neighborY * 4 + neighborX];
      if (neighbor.numPointsX < numPointsX) { // only put imaginary points if I am more refined than my neighbor
        int numPointsBoundary = max(neighbor.numPointsX, numPointsX);
        float spacingBoundaryX = min(neighbor.spacingX, spacingX);
        float boundaryY = neighbor.startY + neighbor.spacingY * (neighbor.numPointsY - 1);
        int i = 1;
        for (; startX + i * spacingBoundaryX <= neighbor.startX + (neighbor.numPointsX - 1) * neighbor.spacingX; ++i) {
          ellipse(startX + i * spacingBoundaryX, boundaryY, PointSize, PointSize);
          if (DrawMesh && coordX == SelectX && coordY == SelectY) {
            line(startX + i * spacingBoundaryX, boundaryY, startX + i * spacingBoundaryX, startY);
            line(startX + i * spacingBoundaryX, boundaryY, startX + (i - 1) * spacingBoundaryX, boundaryY);
          }          
        }
        if (DrawMesh && coordX == SelectX && coordY == SelectY) {
          line(startX, boundaryY, startX, startY);
        }
        boolean meFirst = true;
        /* check the right neighbor of the neighbor */        
        if (neighborX < 3) {
          int otherNeighborX = neighborX + 1;
          int otherNeighborY = neighborY;
          Block otherNeighbor = Blocks[otherNeighborY * 4 + otherNeighborX];
          int ratio1 = numPointsX / neighbor.numPointsX;
          int ratio2 = otherNeighbor.numPointsY / neighbor.numPointsY;
          if ((ratio1 < ratio2) || (ratio1 == ratio2 && (coordX + coordY * 4) > (otherNeighborX + otherNeighborY * 4))) {
            boundaryY = otherNeighbor.startY + (otherNeighbor.numPointsY - 1) * otherNeighbor.spacingY;
            meFirst = false;
          }
        }
        if (meFirst) {
          for (; i < numPointsBoundary; ++i) {
            ellipse(startX + i * spacingBoundaryX, boundaryY, PointSize, PointSize);
            if (DrawMesh && coordX == SelectX && coordY == SelectY) {
              line(startX + i * spacingBoundaryX, boundaryY, startX + i * spacingBoundaryX, startY);
              line(startX + i * spacingBoundaryX, boundaryY, startX + (i - 1) * spacingBoundaryX, boundaryY);
            }            
          }
        } else {
          for (; i < numPointsBoundary; ++i) {
            ellipse(startX + i * spacingBoundaryX, boundaryY, PointSize, PointSize);
            if (DrawMesh && coordX == SelectX && coordY == SelectY) {
              line(startX + i * spacingBoundaryX, boundaryY, startX + i * spacingBoundaryX, startY);
              line(startX + i * spacingBoundaryX, boundaryY, startX + (i - 1) * spacingBoundaryX, boundaryY);
            }            
          }
        }
      }
    }
    // down
    if (coordY < 3) {
      int neighborX = coordX;
      int neighborY = coordY + 1;
      Block neighbor = Blocks[neighborY * 4 + neighborX];
      if (neighbor.numPointsX < numPointsX) {
        int numPointsBoundary = max(neighbor.numPointsX, numPointsX);
        float spacingBoundaryX = min(neighbor.spacingX, spacingX);
        float boundaryY = neighbor.startY;
        for (int i = 1; i < numPointsBoundary; ++i) {
          ellipse(startX + i * spacingBoundaryX, boundaryY, PointSize, PointSize);
          if (DrawMesh && coordX == SelectX && coordY == SelectY) {
            line(startX + i * spacingBoundaryX, boundaryY, startX + i * spacingBoundaryX, startY + (numPointsY - 1) * spacingY);
            line(startX + i * spacingBoundaryX, boundaryY, startX + (i - 1) * spacingBoundaryX, boundaryY);
          }          
        }
        if (DrawMesh && coordX == SelectX && coordY == SelectY) {
          line(startX, boundaryY, startX, startY + (numPointsY - 1) * spacingY);
        }
      }      
    }    
    // draw the selection rectangle
    fill(100, 255, 100, 100);
    if (coordX == SelectX && coordY == SelectY) {
      rect(startX, startY, (numPointsX - 1) * spacingX, (numPointsY - 1) * spacingY);
    }
  }
  
  void refine() {
    ++level;
    if (level % 2 == 1) {
      numPointsX *= 2;
      spacingX /= 2;
    } else {
      numPointsY *= 2;
      spacingY /= 2;
    }    
  }  
  
  void undoRefine() {    
    if (level % 2 == 1) {
      numPointsX /= 2;
      spacingX *= 2;
    } else {
      numPointsY /= 2;
      spacingY *= 2;
    }
    --level;
  }
}

Block[] Blocks;

void setup() {
  size(1000, 1000);
  Blocks = new Block[16];
  for (int i = 0; i < Blocks.length; ++i) {
    Blocks[i] = new Block();
  }
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      Blocks[j * 4 + i].startX = LeftMargin + i * 2 * Spacing;
      Blocks[j * 4 + i].startY = TopMargin + j * 2 * Spacing;
      Blocks[j * 4 + i].coordX = i;
      Blocks[j * 4 + i].coordY = j;
    }
  }
}

void keyPressed() {  
  if (keyCode == UP) {
    SelectY = (SelectY - 1);
    if (SelectY < 0) { SelectY = 3; }
  } else if (keyCode == DOWN) {
    SelectY = (SelectY + 1);
    if (SelectY > 3) { SelectY = 0; }
  } else if (keyCode == LEFT) {
    SelectX = (SelectX - 1);
    if (SelectX < 0) { SelectX = 3; }
  } else if (keyCode == RIGHT) {
    SelectX = (SelectX + 1);
    if (SelectX > 3) { SelectX = 0; }
  } else if (keyCode == ENTER) {
    Blocks[SelectY * 4 + SelectX].refine();
  } else if (keyCode == BACKSPACE) {
    Blocks[SelectY * 4 + SelectX].undoRefine();
  } else if (key == ' ') { // space
    DrawMesh = !DrawMesh;    
  }  
}

void draw() {
  background(255, 255, 255);   
  for (int i = 0; i < Blocks.length; ++i) {
    Blocks[i].Draw2();    
  }
  for (int i = 0; i < Blocks.length; ++i) {
    Blocks[i].Draw1();    
  }
  if (DrawMesh) {
    for (int i = 0; i < Blocks.length; ++i) {
      Blocks[i].DrawMesh();    
    }
  }
  
}
