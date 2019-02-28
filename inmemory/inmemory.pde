int LeftMargin = 40;
int TopMargin = 40;
int Spacing = 100;
int PointSize = 12;
int SelectX = 0;
int SelectY = 0;

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
  
  void DrawMesh(float fromX, int numStepsX, float spacingX, float fromY, int numStepsY, float spacingY) {    
    for (int i = 0; i < numStepsX; ++i) {
      line(fromX + i * spacingX, fromY, fromX + i * spacingX, fromY + (numStepsY - 1) * spacingY);      
    }
    for (int j = 0; j < numStepsY; ++j) {
      line(fromX, fromY + j * spacingY, fromX + (numStepsX - 1) * spacingX, fromY + j * spacingY);
    }
  }
  
  void DrawRealPoints() {
    fill(0, 36, 181);
    for (int i = 0; i < numPointsX; ++i) {
      for (int j = 0; j < numPointsY; ++j) {
        ellipse(startX + i * spacingX, startY + j * spacingY, PointSize, PointSize);
      }
    }
    if (coordX == SelectX && coordY == SelectY) {
      DrawMesh(startX, numPointsX + 1, spacingX, startY, numPointsY + 1, spacingY);
    }
  }
  
  void DrawVirtualPointsAndMeshRight() {
    fill(181, 36, 0);    
    if (coordX < 3) {
      Block neighborRight = Blocks[coordY * 4 + (coordX + 1)];
      float leftX = startX + spacingX * (numPointsX - 1);
      float rightX = neighborRight.startX;      
      /* draw the virtual points */
      int j = 0;
      if (numPointsY >= neighborRight.numPointsY) { // generate points on the right side of the virtual block
        for (; startY + j * spacingY <= startY + (numPointsY - 1) * spacingY; ++j) { // NOTE: some of the points are real points (so we are overdrawing here)
          ellipse(rightX, startY + j * spacingY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          DrawMesh(leftX, 2, (rightX - leftX), startY, j, spacingY);
        }
      } else if (numPointsY < neighborRight.numPointsY) { // generate points on the left side of the virtual block
        for (; neighborRight.startY + j * neighborRight.spacingY <= startY + (numPointsY - 1) * spacingY; ++j) { // NOTE: some of the points are real points (so we are overdrawing here)
          ellipse(leftX, neighborRight.startY + j * neighborRight.spacingY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          DrawMesh(leftX, 2, (rightX - leftX), neighborRight.startY, j, neighborRight.spacingY);
        }
      }
      
    }
  } // end DrawVirtualPointsAndMeshRight
  
  void DrawVirtualPointsAndMeshDown() {
    fill(181, 36, 0);
    if (coordY < 3) {
      Block neighborDown = Blocks[(coordY + 1) * 4 + coordX];
      float topY = startY + spacingY * (numPointsY - 1);
      float bottomY = neighborDown.startY;
      /* draw the virtual points */
      int i = 0;
      if (numPointsX >= neighborDown.numPointsX) { // generate points on the bottom side of the virtual block
        for (; startX + i * spacingX <= startX + (numPointsX - 1) * spacingX; ++i) { // NOTE: some of the points are real points (so we are overdrawing here)
          ellipse(startX + i * spacingX, bottomY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          DrawMesh(startX, i, spacingX, topY, 2, (bottomY - topY));
        }
      } else if (numPointsX < neighborDown.numPointsX) { // generate points on the top side of the virtual block
        for (; neighborDown.startX + i * neighborDown.spacingX <= startX + (numPointsX - 1) * spacingX; ++i) { // NOTE: some of the points are real points (so we are overdrawing here)
          ellipse(neighborDown.startX + i * neighborDown.spacingX, topY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          DrawMesh(neighborDown.startX, i, neighborDown.spacingX, topY, 2, (bottomY - topY));
        }
      }
    }
  } // end DrawVirtualPointsAndMeshDown
  
  void DrawVirtualPointsAndMeshRightDown() {
    fill(181, 36, 0);
    Block neighborRight = coordX < 3 ? Blocks[coordX + 1 + coordY * 4] : null;
    Block neighborBottom = coordY < 3 ? Blocks[coordX + (coordY + 1) * 4] : null;          //<>//
    float topY = startY + (numPointsY - 1) * spacingY;    
    float leftX = startX + (numPointsX - 1) * spacingX;
    if (neighborRight == null && neighborBottom == null) {
      return;
    }
    boolean bottomGoesFirst = (neighborRight == null && neighborBottom != null) ||
                              (neighborRight != null && neighborBottom != null && float(neighborRight.numPointsY) / numPointsY < float(neighborBottom.numPointsX) / numPointsX);
    boolean rightGoesFirst =  (neighborRight != null && neighborBottom == null) ||
                              (neighborRight != null && neighborBottom != null && float(neighborRight.numPointsY) / numPointsY >= float(neighborBottom.numPointsX) / numPointsX);                                                          
    if (bottomGoesFirst) { // the bottom one goes first
      if (neighborBottom != null && neighborBottom.numPointsX >= numPointsX) {        
        int i = neighborBottom.numPointsX - 1;
        for (; neighborBottom.startX + i * neighborBottom.spacingX > startX + (numPointsX - 1) * spacingX; --i) {            
          ellipse(neighborBottom.startX + i * neighborBottom.spacingX, topY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          float bottomY = neighborBottom.startY;
          DrawMesh(leftX, neighborBottom.numPointsX - i + 1, neighborBottom.spacingX, topY, 2, (bottomY - topY));
        } //<>//
        leftX = neighborBottom.startX + (neighborBottom.numPointsX - 1) * neighborBottom.spacingX;
      }
      if (neighborRight != null && neighborRight.numPointsY >= numPointsY) {        
        int j = neighborRight.numPointsY - 1; 
        for (; neighborRight.startY + j * neighborRight.spacingY > startY + (numPointsY - 1) * spacingY; --j) {
          ellipse(leftX, neighborRight.startY + j * neighborRight.spacingY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          float rightX = neighborRight.startX;
          DrawMesh(leftX, 2, (rightX - leftX), topY, neighborRight.numPointsY - j + 1, neighborRight.spacingY);
        }
      }
    } else if (rightGoesFirst) { // the right one goes first
      if (neighborRight != null && neighborRight.numPointsY >= numPointsY) {
        int j = neighborRight.numPointsY - 1;                 
        for (; neighborRight.startY + j * neighborRight.spacingY > startY + (numPointsY - 1) * spacingY; --j) {
          ellipse(leftX, neighborRight.startY + j * neighborRight.spacingY, PointSize, PointSize);
        }        
        if (coordX == SelectX && coordY == SelectY) {
          float rightX = neighborRight.startX;
          DrawMesh(leftX, 2, (rightX - leftX), topY, neighborRight.numPointsY - j + 1, neighborRight.spacingY);
        }
        topY = neighborRight.startY + (neighborRight.numPointsY - 1) * neighborRight.spacingY;
      }
      if (neighborBottom != null && neighborBottom.numPointsX >= numPointsX) {
        int i = neighborBottom.numPointsX - 1;
        for (; neighborBottom.startX + i * neighborBottom.spacingX > startX + (numPointsX - 1) * spacingX; --i) {
          ellipse(neighborBottom.startX + i * neighborBottom.spacingX, topY, PointSize, PointSize);
        }
        if (coordX == SelectX && coordY == SelectY) {
          float bottomY = neighborBottom.startY;
          DrawMesh(leftX, neighborBottom.numPointsX - i + 1, neighborBottom.spacingX, topY, 2, (bottomY - topY));
        }
      }
    }
  }
  
  void DrawSelection() {
    // draw the selection rectangles
    if (coordX == SelectX && coordY == SelectY) {
      fill(100, 255, 100, 100);      
      rect(startX, startY, (numPointsX - 1) * spacingX, (numPointsY - 1) * spacingY);      
      fill(255, 100, 100, 100);
      rect(startX + (numPointsX - 1) * spacingX, startY, spacingX, (numPointsY - 1) * spacingY);
      fill(100, 100, 255, 100);
      rect(startX, startY + (numPointsY - 1) * spacingY, (numPointsX - 1) * spacingX, spacingY);
      fill(255, 170, 100, 100);
      rect(startX + (numPointsX - 1) * spacingX, startY + (numPointsY - 1) * spacingY, spacingX, spacingY);
    }
    
  }
  
  void Draw() {    
    DrawVirtualPointsAndMeshRight();
    DrawVirtualPointsAndMeshDown();
    DrawVirtualPointsAndMeshRightDown();
    DrawRealPoints();    
    DrawSelection();
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
  }
}

void draw() {
  background(255, 255, 255);   
  for (int i = 0; i < Blocks.length; ++i) {
  //  Blocks[i].Draw2();    
  }
  for (int i = 0; i < Blocks.length; ++i) {
//    Blocks[i].Draw1();    
  }
  for (int i = 0; i < Blocks.length; ++i) {
    Blocks[i].Draw();    
  }  
}
