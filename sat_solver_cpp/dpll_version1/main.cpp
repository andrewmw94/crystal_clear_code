#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>

#define UNASSIGNED_LIT -1
#define LIT_DOES_NOT_OCCUR -2

int numSelections = 0;
int maxBestCount = 0;
int numFail2Cls = 0;
int numPosLitSel = 0;

//For 150, the line is roughly 630 clauses. 4.2
//For 100, the line is roughly 420 clauses. 4.2
bool guessSAT;

class clause {
public:
    std::vector<int> literals;
    clause(int n = 0) : literals(n) {
    }
};

bool DPLL(std::vector<clause>, const int, std::vector<int>*);
void unitProp(std::vector<clause>& clauses, std::vector<int> *assignments);
void pureLitAssign(std::vector<clause>& clauses, std::vector<int> *assignments);

//remove clauses that contain `a` and `~a`
void filterClauses(std::vector<clause>& clauses) {
    for (int i = 0; i < clauses.size(); i++) {
myDirtyGOTOLable:
        for (int j = 0; j < clauses[i].literals.size() - 1; j++) {
            for (int k = j + 1; k < clauses[i].literals.size(); k++) {
                if (clauses[i].literals[j] == -clauses[i].literals[k]) {

                    if (i == clauses.size() - 1) {
                        clauses.pop_back();
                        return;
                    }
                    else {
                        clauses[i] = clauses[clauses.size() - 1];
                        clauses.pop_back();
                    }
                    goto myDirtyGOTOLable;
                }
            }
        }
    }
}

void setLitDoesNotOccur(std::vector<clause>& clauses, std::vector<int> *assignments) {
    //Vars start at 1
    for (int k = 1; k < assignments->size(); k++) {
        bool occur = false;
        for (int i = 0; i < clauses.size(); i++) {
            for (int j = 0; j < clauses[i].literals.size(); j++) {
                if (std::abs(clauses[i].literals[j]) == k) {
                    occur = true;
                    goto myOtherDirtyGOTOLable;
                }
            }
        }
myOtherDirtyGOTOLable:
        if (!occur)
            assignments->at(k) = LIT_DOES_NOT_OCCUR;
    }
}

int main(int argc, char** argv) {
    std::ifstream myfile;
    std::string line;
    std::vector<clause> instance(0);
    int numVars = 0, numClauses = 0;

    if (argc != 2) {
        std::cout << "please call 'DPLL <filename>'" << std::endl;
        return 1;
    }

    std::string fileName = argv[1];

    myfile.open(fileName);
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            if (line.length() == 0 || line[0] == 'c')
                continue;
            if (line[0] == 'p') {
                std::string nums = line.substr(6, line.size());
                std::istringstream is(nums);
                std::vector<int> tmpVec = (std::vector<int>(std::istream_iterator<int>(is), std::istream_iterator<int>()));
                numVars = tmpVec[0];
                numClauses = tmpVec[1];
            }
            else {
                clause cls(0);
                std::istringstream is(line);
                std::vector<int> tmpVec = (std::vector<int>(std::istream_iterator<int>(is), std::istream_iterator<int>()));

                for (int i = 0; i < tmpVec.size() - 1; i++) {
                    cls.literals.push_back(tmpVec[i]);
                }
                instance.push_back(cls);
            }
        }

        std::vector<int> assignments;
        assignments.resize(numVars + 1, UNASSIGNED_LIT);

        filterClauses(instance);
        setLitDoesNotOccur(instance, &assignments);
        //std::cout << "setting guess sat: "<<numVars<<", "<<instance.size() << std::endl;
        if (4.2*numVars > instance.size())
            guessSAT = true;
        else
            guessSAT = false;

        bool satisfiable = DPLL(instance, numVars, &assignments);

        if (satisfiable) {
            std::cout << "SAT" << std::endl;
            std::cout << "Num Selections: " << numSelections << std::endl;
            //std::cout << "Num Primary Selections: " << maxBestCount << std::endl;
            //std::cout << "Num Fail 2Clause: " << numFail2Cls << std::endl;
            //std::cout << "Num Pos Lit Sel: " << numPosLitSel << std::endl;


            /*
            std::cout << "Solution: " << std::endl;
            //vars start at 1
            for (int i = 1; i < assignments.size(); i++) {
            if (assignments[i] == 1) {
            std::cout << i << " ";
            }
            else {
            std::cout << -i << " ";
            }
            }
            */
        }
        else {
            std::cout << "UNSAT" << std::endl;
            std::cout << "Num Selections: " << numSelections << std::endl;
            //std::cout << "Num Primary Selections: " << maxBestCount << std::endl;
            //std::cout << "Num Fail 2Clause: " << numFail2Cls << std::endl;
            //std::cout << "Num Pos Lit Sel: " << numPosLitSel << std::endl;
        }
        std::cout << std::endl;

        myfile.close();
    }
    else {
        std::cout << "Unable to open file";
    }

    return 0;
}

//returns true iff all clauses are litterals and the assignment matches the literals
bool sat(std::vector<clause>& clauses, std::vector<int> *assignments) {
    for (auto cls : clauses) {
        if (cls.literals.size() == 1) {
            if ((cls.literals[0] > 0 && assignments->at(cls.literals[0]) == 1) || (cls.literals[0] < 0 && assignments->at(-cls.literals[0]) == 0))
                continue;
        }
        return false;
    }
    return true;
}

//return true iff clauses contains an empty clause
bool unsat(std::vector<clause>& clauses) {
    for (auto cls : clauses) {
        if (cls.literals.size() == 0)
            return true;
    }
    return false;
}

int selectLit(std::vector<clause>& clauses, std::vector<int> *assignments) {

    numSelections++;


    if (guessSAT) {
        std::vector<int> numPosOccur;
        std::vector<int> numNegOccur;
        numPosOccur.resize(assignments->size(), 0);
        numNegOccur.resize(assignments->size(), 0);
        for (int i = 0; i < clauses.size(); i++) {
            if (clauses[i].literals.size() == 2) {
                if (clauses[i].literals[0] > 0)
                    numPosOccur[clauses[i].literals[0]]++;
                else
                    numNegOccur[-clauses[i].literals[0]]++;
                if (clauses[i].literals[1] > 0)
                    numPosOccur[clauses[i].literals[1]]++;
                else
                    numNegOccur[-clauses[i].literals[1]]++;
            }
        }
        int maxIndex = 1;
        for (int i = 2; i < assignments->size(); i++) {
            if (numPosOccur[i] + numNegOccur[i] > numPosOccur[maxIndex] + numNegOccur[maxIndex]) {
                maxIndex = i;
            }
        }
        if (assignments->at(maxIndex) != UNASSIGNED_LIT) {
            //return maxIndex;
            for (int i = 0; i < clauses.size(); i++) {
                for(int j=0; j < clauses[i].literals.size(); j++) {
                    if (clauses[i].literals[j] > 0)
                        numPosOccur[clauses[i].literals[j]]+=32>>clauses[i].literals.size();
                    else
                        numNegOccur[-clauses[i].literals[j]]+=32>>clauses[i].literals.size();
                }

            }
            maxIndex = 1;
            for (int i = 2; i < assignments->size(); i++) {
                if (numPosOccur[i] + numNegOccur[i] > numPosOccur[maxIndex] + numNegOccur[maxIndex]) {
                    maxIndex = i;
                }
            }
        }

        if (numPosOccur[maxIndex] >= numNegOccur[maxIndex])
            return maxIndex;
        return -maxIndex;
    }
    else {
        std::vector<int> numPosOccur;
        std::vector<int> numNegOccur;
        numPosOccur.resize(assignments->size(), 0);
        numNegOccur.resize(assignments->size(), 0);
        for (int i = 0; i < clauses.size(); i++) {
            if (clauses[i].literals.size() == 2) {
                if (clauses[i].literals[0] > 0)
                    numPosOccur[clauses[i].literals[0]]++;
                else
                    numNegOccur[-clauses[i].literals[0]]++;
                if (clauses[i].literals[1] > 0)
                    numPosOccur[clauses[i].literals[1]]++;
                else
                    numNegOccur[-clauses[i].literals[1]]++;
            }
        }
        int maxIndex = 1;
        int secondMaxIndex = 1;
        for (int i = 2; i < assignments->size(); i++) {
            if (numPosOccur[i] + numNegOccur[i] > numPosOccur[maxIndex] + numNegOccur[maxIndex]) {
                secondMaxIndex = maxIndex;
                maxIndex = i;
            }
        }

        if (assignments->at(maxIndex) != UNASSIGNED_LIT) {
            for (int i = 0; i < clauses.size(); i++) {
                for (int j = 0; j < clauses[i].literals.size(); j++) {
                    if (assignments->at(std::abs(clauses[i].literals[j])) == UNASSIGNED_LIT)
                        return std::abs(clauses[i].literals[j]);
                }
            }
        }

        std::vector<clause> clausesCpy1 = clauses;
        std::vector<int> assignmentsCpy = *assignments;
        std::vector<clause> clausesCpy2 = clauses;
        std::vector<int> assignmentsCpy2 = *assignments;
        clause litClause(1);

        if (numPosOccur[maxIndex] >= numNegOccur[maxIndex])
            litClause.literals[0] = maxIndex;
        else
            litClause.literals[0] = -maxIndex;
        clause tmp = clausesCpy1[0];
        clausesCpy1[0] = litClause;
        clausesCpy1.push_back(tmp);
        unitProp(clausesCpy1, &assignmentsCpy);

        if (numPosOccur[secondMaxIndex] >= numNegOccur[secondMaxIndex])
            litClause.literals[0] = secondMaxIndex;
        else
            litClause.literals[0] = -secondMaxIndex;
        tmp = clausesCpy2[0];
        clausesCpy2[0] = litClause;
        clausesCpy2.push_back(tmp);
        unitProp(clausesCpy2, &assignmentsCpy);
        if (clausesCpy2.size() >= clausesCpy1.size()) {
            maxBestCount++;
            if (numPosOccur[maxIndex] >= numNegOccur[maxIndex])
                return maxIndex;
            return -maxIndex;
        }

        if (numPosOccur[secondMaxIndex] >= numNegOccur[secondMaxIndex])
            return secondMaxIndex;
        return -secondMaxIndex;
    }
}

//Find clauses that contain only one variable. Assign that variable if not already assigned and simplfy
//i.e., remove the clauses that contain 'a' and remove '~a' from all clauses.
void unitProp(std::vector<clause>& clauses, std::vector<int> *assignments) {
    bool changed;
    int changedLit;

    //repeat until no changes are made:
    do {
        changed = false;
        changedLit = -1;

        //For each unit clause, if the variable is unassigned, assign it and set changed
        for (int i = 0; i < clauses.size(); i++) {
            if (clauses[i].literals.size() == 1) {
                if (assignments->at(std::abs(clauses[i].literals[0])) == UNASSIGNED_LIT) {
                    if (clauses[i].literals[0] > 0)
                        assignments->at(std::abs(clauses[i].literals[0])) = 1;
                    else
                        assignments->at(std::abs(clauses[i].literals[0])) = 0;
                    changed = true;
                    changedLit = clauses[i].literals[0];
                    break;
                }
            }
        }
        //If a variable is changed, we remove clauses that contain that variable
        //and remove the negation of that variable from the remaining clauses
        if (changed) {
            for (int i = 0; i < clauses.size(); i++) {
                for (int j = 0; j < clauses[i].literals.size(); j++) {
                    if (clauses[i].literals[j] == changedLit) {
                        if (i == clauses.size() - 1) {
                            clauses.pop_back();
                        }
                        else {
                            clauses[i] = clauses[clauses.size() - 1];
                            clauses.pop_back();
                        }
                        i--;
                        break;
                    }
                    else if (clauses[i].literals[j] == -changedLit) {
                        if (j == clauses[i].literals.size() - 1) {
                            clauses[i].literals.pop_back();
                        }
                        else {
                            clauses[i].literals[j] = clauses[i].literals[clauses[i].literals.size() - 1];
                            clauses[i].literals.pop_back();
                        }
                        j--;
                    }
                }
            }
        }
    } while (changed);
}

//Find literals that only appear in one polarity. Set the variable to the cooresponding assingment and remove
//clauses containing that literal.
void pureLitAssign(std::vector<clause>& clauses, std::vector<int> *assignments) {
    //vars start at 1
    for (int k = 1; k < assignments->size(); k++) {
        if (assignments->at(k) == LIT_DOES_NOT_OCCUR)
            continue;
        bool appearTrue = false;
        bool appearFalse = false;
        for (auto cls : clauses) {
            for (auto l : cls.literals) {
                if (l == k) {
                    appearTrue = true;
                    if (appearFalse)
                        goto myLastDirtyGOTOLable;
                }
                else if (l == -k) {
                    appearFalse = true;
                    if (appearTrue)
                        goto myLastDirtyGOTOLable;
                }

            }
        }
        if (appearTrue && !appearFalse) {
            assignments->at(k) = 1;
            for (int i = 0; i < clauses.size(); i++) {
                for (int j = 0; j < clauses[i].literals.size(); j++) {
                    if (clauses[i].literals[j] == k) {
                        if (i == clauses.size() - 1) {
                            clauses.pop_back();
                        }
                        else {
                            clauses[i] = clauses[clauses.size() - 1];
                            clauses.pop_back();
                        }
                        i--;
                        break;
                    }
                }
            }
        }
        else if (!appearTrue && appearFalse) {
            assignments->at(k) = 0;
            for (int i = 0; i < clauses.size(); i++) {
                for (int j = 0; j < clauses[i].literals.size(); j++) {
                    if (clauses[i].literals[j] == -k) {
                        if (i == clauses.size() - 1) {
                            clauses.pop_back();
                        }
                        else {
                            clauses[i] = clauses[clauses.size() - 1];
                            clauses.pop_back();
                        }
                        i--;
                        break;
                    }
                }
            }
        }
myLastDirtyGOTOLable:
        ;
    }
}

bool DPLL(std::vector<clause> clauses, const int numVars, std::vector<int> *assignments) {

    if (sat(clauses, assignments)) {
        return true;
    }
    else if (unsat(clauses)) {
        return false;
    }
    else {
        std::vector<int> assignCopy = *assignments;
        std::vector<clause> clausesCopy = clauses;

        unitProp(clausesCopy, &assignCopy);
        pureLitAssign(clausesCopy, &assignCopy);

        if (sat(clausesCopy, &assignCopy)) {
            *assignments = assignCopy;
            return true;
        }
        else if (unsat(clausesCopy)) {
            return false;
        }

        int lit = selectLit(clausesCopy, &assignCopy);//signed proposition to assign

        if (lit == 0) {
            std::cout << "Fatal error, no literal selected" << std::endl;

            std::cout << "num clauses: " << clausesCopy.size() << std::endl;
            return true;
        }

        clause litClause(1);
        litClause.literals[0] = lit;
        clause tmp = clausesCopy[0];
        clausesCopy[0] = litClause;
        clausesCopy.push_back(tmp);

        int litVal = assignCopy[lit];

        if (DPLL(clausesCopy, numVars, &assignCopy)) {
            *assignments = assignCopy;
            return true;
        }

        litClause.literals[0] = -lit;
        clausesCopy[0] = litClause;
        if (DPLL(clausesCopy, numVars, &assignCopy)) {
            *assignments = assignCopy;
            return true;
        }
        return false;

    }
}
