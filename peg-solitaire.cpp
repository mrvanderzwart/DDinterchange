#include "dd_exchange.hpp"

#include <iostream>
#include <string>

using namespace interchange;

enum Square {
    NN = 0, 
    OO = 1, 
    EE = 2, 
};

#define N 5
Square BOARD[N][N] = {
    {NN,NN,OO,NN,NN},
    {NN,NN,OO,NN,NN},
    {OO,OO,EE,OO,OO},
    {NN,NN,OO,NN,NN},
    {NN,NN,OO,NN,NN}
};

struct PartialRelation
{  
  dd MoveRelation;
  vset RelationVars;
};

class PegSolitaire
{
  public:
    PegSolitaire();
    void CreateInitialBoard();
    void CreateWinningBoards();
    void CreateMoveRelation();
    void PrintBoards(dd &D, std::string name);
    dd BFS(dd &FixPoint);
    
    dd InitialBoard;
    dd WinningBoards;
    dd MoveRelation;
    ddvar Coor2Var[N][N];
    vset SourceVars;
    vset TargetVars;
    vset RelationVars;
    std::vector<vset> Triples;
    std::vector<PartialRelation> Relations;
};

PegSolitaire::PegSolitaire()
{
  ddvar var = 0;

  for (int i = 0; i < N; i++) 
  {
    for (int j = 0; j < N; j++) 
    {
      if (BOARD[i][j] == NN) 
        continue;

      ddvar source = var++;  
      ddvar target = var++;

      SourceVars.add(source);  
      TargetVars.add(target);
      
			Coor2Var[i][j] = source;
    }
  }

  RelationVars.add(SourceVars);
  RelationVars.add(TargetVars);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (BOARD[i][j] == NN)
        continue;
        
      if (j+2 < N && BOARD[i][j+1] != NN && BOARD[i][j+2] != NN) 
      {
        vset Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i][j+1]);
        Triple.add(Coor2Var[i][j+2]);
        Triples.push_back(Triple);
      }

      if (i+2 < N && BOARD[i+1][j] != NN && BOARD[i+2][j] != NN) 
      {
        vset Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i+1][j]);
        Triple.add(Coor2Var[i+2][j]);
        Triples.push_back(Triple);
      }    
    }
  }
}

void 
PegSolitaire::CreateInitialBoard()
{
  InitialBoard = dd::ddOne();

  for (int i = 0; i < N; i++) 
  {
    for (int j = 0; j < N; j++) 
    {
      if (BOARD[i][j] == NN)
        continue;
      ddvar square = Coor2Var[i][j];
      if (BOARD[i][j] == EE)
        InitialBoard &= !dd::ddVar(square);
      else 
        InitialBoard &= dd::ddVar(square);
    }
  }
}

void PegSolitaire::CreateWinningBoards() 
{
  WinningBoards = dd::ddZero(); 

  for (ddvar square : SourceVars.toVector()) 
  {
    dd Board = dd::ddOne();  
    for (ddvar square2 : SourceVars.toVector()) 
    {
      if (square2 == square) 
      {
        Board &= dd::ddVar(square2);
      }
      else 
        Board &= !dd::ddVar(square2);
    }
   
    WinningBoards |= Board;
  }
}

void PegSolitaire::CreateMoveRelation()
{
  for (vset &Triple : Triples)
  {
    std::vector<ddvar> V = Triple.toVector();
    
    PartialRelation Partial;
    for (ddvar var : V)
    {
      ddvar source = var;
      ddvar target = var+1;
    
      Partial.RelationVars.add(source);
      Partial.RelationVars.add(target);
    }   
    Partial.MoveRelation = dd::ddZero();
    
    dd move1 =  dd::ddVar(V[0]) & !dd::ddVar(V[0]+1) &
                dd::ddVar(V[1]) & !dd::ddVar(V[1]+1) &
               !dd::ddVar(V[2]) &  dd::ddVar(V[2]+1);
               
    Partial.MoveRelation |= move1;
    
    dd move2 = (!dd::ddVar(V[0])) &  dd::ddVar(V[0]+1) &
                 dd::ddVar(V[1])  & !dd::ddVar(V[1]+1) &
                 dd::ddVar(V[2])  & !dd::ddVar(V[2]+1);
                
    Partial.MoveRelation |= move2;
    Relations.push_back(Partial);
  }
  size_t total_nodes = 0;
  MoveRelation = dd::ddZero();

  for (PartialRelation& Partial : Relations)
  {
    dd Rel = Partial.MoveRelation;
    
    for (ddvar var : SourceVars.toVector())
    {
      if (Partial.RelationVars.contains(var))
        continue;

      Rel &= (dd::ddVar(var) - dd::ddVar(var+1)); 
    }
    MoveRelation |= Rel;
    total_nodes += Partial.MoveRelation.NodeCount();
  }
  printf("Partial relations have a total of %ld nodes\n", total_nodes);
}

dd 
PegSolitaire::BFS(dd &FixPoint)
{
  dd next = FixPoint;
  dd visited = dd::ddZero();
  while (next != dd::ddZero())
  {
    visited |= next;
    next = next.RelNext(MoveRelation, RelationVars);
  }
  return visited;
}

void printboards(void *ctx, std::vector<ddvar> VA, auto values, int count)
{
  printf("--------------------\n");
  PegSolitaire *Game = (PegSolitaire *) ctx;
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (BOARD[i][j] == NN)
        printf(" , ");
      else
      {
        ddvar var = Game->Coor2Var[i][j];
        for (int k = 0; k < count; k++)
        {
          if (var == VA[k])
          {
            printf("%s, ", (values[k] ? "1" : "0"));
            break;
          }
        }
      }
    }
    printf("\n");
  }
  printf("--------------------\n");
}

void 
PegSolitaire::PrintBoards(dd &D, std::string name) 
{
  printf("Printing boards: %s \n", name.c_str());
  D.SetEnum(SourceVars, printboards, this);
}

int main()
{
  PegSolitaire game;
  game.CreateInitialBoard();
  game.PrintBoards(game.InitialBoard, "initial");
  game.CreateMoveRelation();
  dd all = game.BFS(game.InitialBoard);
  game.InitialBoard = game.InitialBoard.RelNext(game.MoveRelation, game.RelationVars);
  game.PrintBoards(game.InitialBoard, "Next");
}
