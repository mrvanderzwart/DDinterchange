#include "sylvan_dd.h"

using namespace sylvan;
using namespace std;

enum Square 
{
  NN = 0,
  OO = 1,
  EE = 2
};

SddManager* Sdd::Manager = sdd_manager_create(2*((7*7)-(4*4)), 0);

/**
 * Declare callback function for printing boards from BDD package
 */
VOID_TASK_DECL_4(printboard_bdd, void*, BDDVAR*, uint8_t*, int);
#define printboard_bdd TASK(printboard_bdd)

#define N 7
Square BOARD[N][N] = 
{
  {NN,NN,OO,OO,OO,NN,NN,},
  {NN,NN,OO,OO,OO,NN,NN,},
  {OO,OO,OO,OO,OO,OO,OO,},
  {OO,OO,OO,EE,OO,OO,OO,},
  {OO,OO,OO,OO,OO,OO,OO,},
  {NN,NN,OO,OO,OO,NN,NN,},
  {NN,NN,OO,OO,OO,NN,NN,}
};

struct PartialRelation
{  
  Dd MoveRelation;
  DDSET RelationVars;
};

class PegSolitaire
{
  public:
    PegSolitaire();
    void CreateInitialBoard();
    void CreateWinningBoards();
    void CreateMoveRelation();
    void PrintBoards(Dd& D, string name);
    
    Dd InitialBoard;
    Dd WinningBoards;
    Dd MoveRelation;
    DDVAR Coor2Var[N][N];
    DDSET SourceVars;
    DDSET TargetVars;
    DDSET RelationVars;
    vector<DDSET> Triples;
    vector<PartialRelation> Relations;
    
  private:
    SddExtra ex;
};

PegSolitaire::PegSolitaire()
{
  DDVAR var;
  if (DDchoice) //SDD starts at 1
    var = 1;
  else          //BDD starts at 0
    var = 0;
    
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (BOARD[i][j] == NN)
        continue;
        
      DDVAR source = var++;
      DDVAR target = var++;     
      Coor2Var[i][j] = source;
      SourceVars.add(source);
      TargetVars.add(target);
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
        DDSET Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i][j+1]);
        Triple.add(Coor2Var[i][j+2]);
        Triples.push_back(Triple);
      }

      if (i+2 < N && BOARD[i+1][j] != NN && BOARD[i+2][j] != NN) 
      {
        DDSET Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i+1][j]);
        Triple.add(Coor2Var[i+2][j]);
        Triples.push_back(Triple);
      }    
    }
  }

}

void PegSolitaire::CreateInitialBoard()
{
  InitialBoard = Dd::ddOne(); 
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (BOARD[i][j] == NN)
        continue;
        
      DDVAR square = Coor2Var[i][j];
      if (BOARD[i][j] == EE)
        InitialBoard = InitialBoard & !Dd(square);
      else
        InitialBoard = InitialBoard & Dd(square);
    }
  }
}

void PegSolitaire::CreateWinningBoards() 
{
  WinningBoards = Dd::ddZero(); 
  for (DDVAR square : SourceVars.toVector()) 
  {
    Dd Board = Dd::ddOne();  
    for (DDVAR square2 : SourceVars.toVector()) 
    {
      if (square2 == square) 
        Board &= Dd(square);
      else 
        Board &= !Dd(square2);
    }
    WinningBoards = WinningBoards | Board;
  }
}

void PegSolitaire::CreateMoveRelation()
{
  for (DDSET &Triple : Triples)
  {
    vector<DDVAR> V = Triple.toVector();
    PartialRelation Partial;
    for (DDVAR var : V)
    {
      Partial.RelationVars.add(var);
      Partial.RelationVars.add(var+1);
    }   
    Partial.MoveRelation = Dd::ddZero();
    
    Dd move1 =  Dd(V[0]) & !Dd(V[0]+1) &
                Dd(V[1]) & !Dd(V[1]+1) &
               !Dd(V[2]) &  Dd(V[2]+1);
               
    Partial.MoveRelation |= move1;
    
    Dd move2 = (!Dd(V[0])) &  Dd(V[0]+1) &
                Dd(V[1]) & !Dd(V[1]+1) &
                Dd(V[2]) & !Dd(V[2]+1);
                
    Partial.MoveRelation |= move2;
    Relations.push_back(Partial);
  }
  size_t total_nodes = 0;
  MoveRelation = Dd::ddZero();
  for (PartialRelation& Partial : Relations)
  {
    Dd Rel = Partial.MoveRelation;
    for (DDVAR var : SourceVars.toVector())
    {
      if (Partial.RelationVars.contains(var))
        continue;
      Rel &= Dd(var).Xnor(Dd(var+1));
    }
    MoveRelation |= Rel;
    total_nodes += Partial.MoveRelation.NodeCount();
  }
  cout << endl << "Partial relations have a total of " << total_nodes << " nodes\n";
}

/**
 * Implementation of callback for printing board from SDD package
 */
void printboard_sdd(int* elements)
{
    cout << "--------------------" << endl;
    int k = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (BOARD[i][j] == NN) {
                cout << " , ";
            } else {
                cout << elements[k] << ", ";
                k++;
            }
        }
        cout << endl;
    }
    cout << "--------------------" << endl;
}

void PegSolitaire::PrintBoards(Dd& D, string name)
{
  LACE_ME;
  cout << endl <<  endl << "Printing boards: "<< name << endl;
  //sylvan_enum(D.GetDD(), SourceVars.GetDD(), printboard, this); //BDD
  ex.set_enum(D.GetDD(), Sdd::Manager, printboard_sdd); //SDD
}

/**
 * Implementation of callback for printing boards from BDD package
 */
VOID_TASK_IMPL_4(printboard_bdd, void*, ctx, BDDVAR*, VA, uint8_t*, values, int, count) 
{
    PegSolitaire *Game = (PegSolitaire *) ctx;
    cout << "--------------------" << endl;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (BOARD[i][j] == NN) {
                cout << " , ";
            } else {
                BDDVAR var = Game->Coor2Var[i][j];
                for (int k = 0; k < count; k++) {
                    if (var == VA[k]) {
                        cout << (values[k] ? "1" : "0") <<", ";
                        break;
                    }
                }
            }
        }
        cout << endl;
    }
    cout << "--------------------" << endl;
}

int main()
{
  /* Init BDD package */
  lace_init(0, 0);
  lace_startup(0, NULL, NULL);
  LACE_ME;
  sylvan_set_sizes(1ULL<<22, 1ULL<<27, 1ULL<<20, 1ULL<<26);
  sylvan_init_package();
  sylvan_init_bdd();
  
  PegSolitaire game;
  game.CreateInitialBoard();
  game.PrintBoards(game.InitialBoard, "Initial Board");
  game.CreateWinningBoards();
  game.PrintBoards(game.WinningBoards, "Winning Boards");
}
