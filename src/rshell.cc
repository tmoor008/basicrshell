#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <vector>
#include <queue>
#include <stdio.h>
#include <cstring>
#include <sys/wait.h>
#include <sys/types.h>

using namespace std;
using namespace boost;


class Connectors    //abstract base class so we can dynamically call run
{
    public:
        virtual ~Connectors()
        {}
        virtual int run(int state) = 0;  
};

class Semicolon : public Connectors
{
    private:
       vector<char *> vctr; 
    public:
    Semicolon(vector<string> v) : vctr(50)  //constructor creates char* vector
    {                                       //which stores one command+params
        vctr.reserve(v.size());             //in a semicolon object
        for (unsigned i = 0; i < v.size(); ++i)
        {
            vctr[i] = const_cast<char*>(v[i].c_str());
        }
        vctr.push_back(NULL);   //makes sure the command ends with a null char
    }
    virtual ~Semicolon()
    {
        unsigned sz = vctr.size();
        for (unsigned i = 0; i < sz; ++i)
        {
            vctr.pop_back();
        }
    }
                           //so execvp can determine the end
    virtual int run(int state)
    {
        char **pointer = &vctr[0];  //points to vctr we'll use for execvp
        
        string ex = "exit";
        string p = pointer[0];
        if (p == ex) // check if the command entered is exit
        {
            state = -1;
            return state;
        }
        
        pid_t c_pid, pid;
        int status;
        c_pid = fork();

        if (c_pid < 0)      //checks if fork failed
        {
            perror("fork failed");
            state = 0; 
            return state;
        }
        else if (c_pid == 0)    //if we're in the child, call execvp
        {
            execvp(pointer[0], pointer);
            perror("execvp failed");
            exit(127);         
        }
        else //if in the parent, wait for child to terminate
        {
            //checks if wait fails
            if ((pid = waitpid(c_pid, &status, 0)) < 0)
            {
                perror("wait failed");
                state = 0;
                return state;
            }
            //checks whether the child succeeded or not
            //returns the state accordingly
            if (status == 0)
            {
                state = 1; //if it succeeded, set the state to true
                return state;
            }
            else
            {
                state = 0;
                return state;
            }
        }
        return 0;
    }
};

class And : public Connectors
{
    private:
       vector<char *> vctr; 
    public:
    And(vector<string> v) : vctr(50)
    {       
        vctr.reserve(v.size()); //store one command+param of type And
        for (unsigned i = 0; i < v.size(); ++i)
        {
            vctr[i] = const_cast<char*>(v[i].c_str());
        }
        vctr.push_back(NULL); //end with null char
    }

    virtual ~And()
    {
        unsigned sz = vctr.size();
        for (unsigned i = 0; i < sz; ++i)
        {
            vctr.pop_back();
        }
    }

    virtual int run(int state)
    {
        if (state != 1) //return if the previous command failed
        {
            return state;
        }    
    
        char **pointer = &vctr[0];  //points to vctr we'll use for execvp
        
        string ex = "exit";
        string p = pointer[0];
        if (p == ex) // check if the command entered is exit
        {
            state = -1;
            return state;
        }
        
        pid_t c_pid, pid;
        int status;
        c_pid = fork();

        if (c_pid < 0)      //checks if fork failed
        {
            perror("fork failed");
            state = 0; 
            return state;
        }
        else if (c_pid == 0)    //if we're in the child, call execvp
        {
            execvp(pointer[0], pointer);
            perror("execvp failed");
            exit(127);         
        }
        else //if in the parent, wait for child to terminate
        {
            //checks if wait fails
            if ((pid = waitpid(c_pid, &status, 0)) < 0)
            {
                perror("wait failed");
                state = 0;
                return state;
            }
            //checks whether the child succeeded or not
            //outputs the state accordingly
            if (status == 0)
            {
                state = 1; //if it succeeded, set the state to true
                return state;
            }
            else
            {
                state = 0;
                return state;
            }
        }
        return 0;    
    }
};

class Or : public Connectors
{
    private:
       vector<char *> vctr; 
    public:
    Or(vector<string> v) : vctr(50) //stores one command+params of type Or
    {       
        vctr.reserve(v.size());
        for (unsigned i = 0; i < v.size(); ++i)
        {
            vctr[i] = const_cast<char*>(v[i].c_str());
        }
        vctr.push_back(NULL); //end with null
    }
    virtual ~Or()
    {
        unsigned sz = vctr.size();
        for (unsigned i = 0; i < sz; ++i)
        {
            vctr.pop_back();
        }
    }

    virtual int run(int state)
    {
        if (state != 0) //return if the previous command succeeded
        {
            return state;
        }    
        
        char **pointer = &vctr[0];  //points to vctr we'll use for execvp
        
        string ex = "exit";
        string p = pointer[0];
        if (p == ex) // check if the command entered is exit
        {
            state = -1;
            return state;
        }
        
        pid_t c_pid, pid;
        int status;
        c_pid = fork();

        if (c_pid < 0)      //checks if fork failed
        {
            perror("fork failed");
            state = 0; 
            return state;
        }
        else if (c_pid == 0)    //if we're in the child, call execvp
        {
            execvp(pointer[0], pointer);
            perror("execvp failed");
            exit(127);         
        }
        else //if in the parent, wait for child to terminate
        {
            //checks if wait fails
            if ((pid = waitpid(c_pid, &status, 0)) < 0)
            {
                perror("wait failed");
                state = 0;
                return state;
            }
            //checks whether the child succeeded or not
            //outputs the state accodingly
            if (status == 0)
            {
                state = 1; //if it succeeded, set the state to true
                return state;
            }
            else
            {
                state = 0;
                return state;
            }
        }
        return 0; 
    }
};



int main()
{
    int w = 0;
    while (w == 0)
    {   
        // extra credit part
        char *username;
        int host;
        if ((username = getlogin()) != NULL) // get the username
        {
            char name[101];
            int len = 100;
            if ((host = gethostname(name, len)) == 0) // get the hostname
            {
                cout << username << "@" << name << "$ ";        
            }
            else
            {
                 cout << "$ ";           //outputs terminal $
            }
        }
        else
        {
            cout << "$ ";
        }
    
        string input;
        getline(cin, input);    //gets user input    

        //containers we'll use for storage
        vector< vector<string> > v;
        vector<string> t;
        vector<Connectors*> objects;
        queue<string> q;        
        
        int column = 0;
        
        //creates tokenizer and char separator to parse input
        typedef tokenizer<char_separator<char> > tokenizer; 
        char_separator<char> sep(" ", ";#|&");
        tokenizer tokens(input, sep);

        bool lastVal = 0;
        bool firstVal = 0;
        
        //checks for placement of connectors
        for (tokenizer::iterator check = tokens.begin(); check != tokens.end(); ++check)
        {
            if (check == tokens.begin())
            {
                if ((*check == "|") || (*check == "&") || (*check == ";"))
                {
                    firstVal = 1;
                }
            }
            tokenizer::iterator count = check;
            ++count;
            if (count == tokens.end())
            {
                if ((*check == "|") || (*check == "&"))
                {                
                    lastVal = 1;
                }    
            }
        }

        //loops again if the input begins or ends with a connector    
        if (firstVal == 1)
        {
            cout << "beginning of input cannot be a connector" << endl;
            continue;
        }

        if (lastVal == 1)
        {
            cout << "end of input cannot be a connector" << endl;
            continue;
        }

        bool flag = 0; //flag to check when to start a new column
        bool wrong = 0;
        //holds commands in a 2d vector

        //pushes commands into 2d vector and pushes connectors into a queue
        for (tokenizer::iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
        {
            tokenizer::iterator temp = itr;
            if (*itr == ";")
            {
                ++temp;
                if (temp != tokens.end())
                {
                    if (*temp == *itr)
                    {
                        cout << "cannot have multiple semicolons" << endl;
                        wrong = 1;
                        break;
                    }
                }
                q.push(*itr);
                column = column + 1;
                flag = 0;   
            }

            else if ((*itr == "|") || (*itr == "&"))   
            {  
                ++temp;
                if (*temp != *itr)
                {
                    cout << "cannot enter single connector" << endl;
                    wrong = 1;
                    break;
                }
                ++temp;
                if (*temp == *itr)
                {
                    cout << "cannot enter 3+ connector" << endl;
                    wrong = 1;
                    break;   
                }
                         
                q.push(*itr); //pushes connector into queue
                column = column + 1; //increments column
                flag = 0;
                ++itr;  //extra increments itr to not test second connector in pair                             
            }
            else if (*itr == "#")
            {
                break;          //if there's a comment, break out of loop
            }
            else
            {
                if (!flag)
                {
                    v.push_back(t); //starts a new column
                    v.at(column).push_back(*itr);
                    flag = 1;
                }
                else
                { //push value into position
                    v.at(column).push_back(*itr);
                }
            }
        }

        //checks the contents of v
        //for (unsigned i = 0; i < v.size(); ++i)
        //{
           //for (unsigned j = 0; j < v.at(i).size(); ++j)
           //{
                //cout << v.at(i).at(j);   
           //}
            
        //}

        //loops if wrong amt of connectors are entered
        if (wrong == 1)
        {
            continue;
        } 
        
        //this part of the code creates a temp vector current which holds
        //a single command+param chunk at a time
        //then determines the connector previous to the command its told to run
        //it then creates the corresponding connector class type object
        //and pushes the new object into a vector of Connectors pointers
        bool first = 1;
        vector<string> current;

        for (unsigned i = 0; i < v.size(); ++i)
        {
            for (unsigned j = 0; j < v.at(i).size(); ++j)
            {
                current.push_back(v.at(i).at(j));     
            }
            if (!q.empty() && first != 1)
            {
                if (q.front() == ";")
                {
                    objects.push_back(new Semicolon(current));
                }

                if (q.front() == "|")
                {   
                    objects.push_back(new Or(current));
                }
            
                if (q.front() == "&")
                {
                    objects.push_back(new And(current));
                }
                q.pop();
            }
            if (first == 1)
            {   
                objects.push_back(new Semicolon(current));
                first = 0;
            }   
            current.clear();
        }

        int beg = 0;
        int durr = 0;

        //this loop goes through the object vector and calls run on each 
        //object, dynamically calling the run of the class type
        //cout << "Size: " << objects.size()  << endl;
        for (unsigned i = 0; i < objects.size(); ++i)
        {
            //cout << "Curr size: " << objects.size() << endl;
            durr = objects.at(i)->run(beg);
            //cout << "State after run: " << durr << endl;
            //check for if exit is called
            if (durr == -1)
            {
                break;
            }
            beg = durr;
        }
        
        //deletes the dynamically allocated memory
        Connectors *p;
        for (vector<Connectors*>::iterator ptr = objects.begin(); ptr != objects.end();
        ++ptr)
        {
            p = *ptr;
            delete p;
        }
        p = NULL;
        
        //makes sure to end program if exit is called
        if (durr == -1)
        {
            break;
        }
    }
    return 0;
}


