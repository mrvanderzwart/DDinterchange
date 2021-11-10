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
/*
#define N 3
Square BOARD[N][N] = {
    {OO,NN,NN},
    {OO,NN,NN},
    {EE,OO,OO}
};
*/
struct PartialRelation
{  
  dd MoveRelation;
  ddset RelationVars;
};

class PegSolitaire
{
  public:
    PegSolitaire();
    void CreateInitialBoard();
    void CreateWinningBoards();
    void CreateMoveRelation();
    void PrintBoards(dd &D, std::string name);
    
    dd InitialBoard;
    dd WinningBoards;
    dd MoveRelation;
    ddvar Coor2Var[N][N];
    ddset SourceVars;
    ddset TargetVars;
    ddset RelationVars;
    std::vector<ddset> Triples;
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
        ddset Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i][j+1]);
        Triple.add(Coor2Var[i][j+2]);
        Triples.push_back(Triple);
      }

      if (i+2 < N && BOARD[i+1][j] != NN && BOARD[i+2][j] != NN) 
      {
        ddset Triple;
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
  InitialBoard = Factory::ddOne();

  for (int i = 0; i < N; i++) 
  {
    for (int j = 0; j < N; j++) 
    {
      if (BOARD[i][j] == NN)
        continue;
      ddvar square = Coor2Var[i][j];
      if (BOARD[i][j] == EE)
        InitialBoard &= !Factory::ddVar(square);
      else 
        InitialBoard &= Factory::ddVar(square);
    }
  }
}

void PegSolitaire::CreateWinningBoards() 
{
  WinningBoards = Factory::ddZero(); 

  for (ddvar square : SourceVars.toVector()) 
  {
    dd Board = Factory::ddOne();  
    for (ddvar square2 : SourceVars.toVector()) 
    {
      if (square2 == square) 
      {
        Board &= Factory::ddVar(square2);
      }
      else 
        Board &= !Factory::ddVar(square2);
    }
   
    WinningBoards |= Board;
  }
}

void PegSolitaire::CreateMoveRelation()
{
  for (ddset &Triple : Triples)
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
    Partial.MoveRelation = Factory::ddZero();
    
    dd move1 =  Factory::ddVar(V[0]) & !Factory::ddVar(V[0]+1) &
                Factory::ddVar(V[1]) & !Factory::ddVar(V[1]+1) &
               !Factory::ddVar(V[2]) &  Factory::ddVar(V[2]+1);
               
    Partial.MoveRelation |= move1;
    
    dd move2 = (!Factory::ddVar(V[0])) &  Factory::ddVar(V[0]+1) &
                 Factory::ddVar(V[1])  & !Factory::ddVar(V[1]+1) &
                 Factory::ddVar(V[2])  & !Factory::ddVar(V[2]+1);
                
    Partial.MoveRelation |= move2;
    Relations.push_back(Partial);
  }
  size_t total_nodes = 0;
  MoveRelation = Factory::ddZero();

  for (PartialRelation& Partial : Relations)
  {
    dd Rel = Partial.MoveRelation;
    
    for (ddvar var : SourceVars.toVector())
    {
      if (Partial.RelationVars.contains(var))
        continue;

      Rel &= (Factory::ddVar(var) - Factory::ddVar(var+1)); 
    }
    MoveRelation |= Rel;
    total_nodes += Partial.MoveRelation.NodeCount();
  }
  printf("Partial relations have a total of %ld nodes\n", total_nodes);
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
  //sdd_save("initial.txt", game.InitialBoard.GetDD());
  game.CreateMoveRelation();
  //game.CreateWinningBoards();
  //sdd_save_as_dot("win.dot", game.WinningBoards.GetDD());
  //printf("%lld \n", game.WinningBoards.SatCount(game.SourceVars));
  //printf("%ld \n", game.WinningBoards.NodeCount());
  //game.PrintBoards(game.InitialBoard, "Init");
  game.InitialBoard = game.InitialBoard.RelNext(game.MoveRelation, game.RelationVars);
  sdd_save_as_dot("aftermove.dot", game.InitialBoard.GetDD());
  game.PrintBoards(game.InitialBoard, "Next");
  //game.PrintBoards(game.WinningBoards, "Win");
}
