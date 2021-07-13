#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <queue>

using namespace std;

template <typename T>
T StringToNumber ( const string &Text ) 
{                              
    stringstream ss(Text);
    T result;
    return ss >> result ? result : 0;
}

template <typename T>
string NumberToString ( T Number )
{
    stringstream ss;
    ss << Number;
    return ss.str();
}


void parse(string &s,vector <string> &parsed_string) {
    stringstream ss(s);
    string buf;
    while (ss >> buf)
	parsed_string.push_back(buf);
}

void trim_right(string &str) {
    size_t endpos = str.find_last_not_of("(");
    if( str.npos != endpos )
	str = str.substr( 0, endpos+1 );
}

struct det {
    double coeff;
    string orbs;
    det() {
        coeff = 0.0;
	orbs = "";
    }
};

void read_input(int &argc, char** &argv, int &err) {
    if (argc != 5) {
        cout << "Error: Need to input Dirac output file and basename for slater output file" << endl;
	cout << "       and the root number you want to convert" << endl;
	err = 1;
    }
}

void write_header(ofstream &slater, int &nmo,string &base_name) {
    slater << "SPINORSLATER" << endl;
    slater << "NOSORT" << endl;
    slater << "CORBITALS {" << endl;
    slater << "QUATERNION_MO" << endl;
    slater << "   MAGNIFY 1" << endl;
    slater << "   NMO " << nmo << endl;
    slater << "   ORBFILE " << base_name << ".orb" << endl;
    slater << "   INCLUDE " << base_name << ".basis" << endl;
    slater << "   CENTERS { USEATOMS } " << endl;
    slater << "}" << endl;
}

int find_nmo(ifstream &dirac, ofstream &slater,string &base_name) {
    string line;
    int nmo = 0;
    while (dirac.good()) {
	getline(dirac,line);
	if (line.find("tot.num.") != line.npos) {
	    vector<string> words;
	    parse(line,words);
	    for (int i = 0; i < words.size(); i++) {
		if (words[i] == "tot.num" or words[i] == "of" or words[i] == "pos.erg" or words[i] == "shells:") continue;
		nmo += StringToNumber<int>(words[i]);
	    }
	}
    }
    nmo *= 2;
    write_header(slater,nmo,base_name);
}

void write_dets(ofstream & slater, vector <det> dets, double &thres) {
    slater << endl;
    int flag = 1;
    if (dets[0].coeff < 0)
	flag = -1;
    //double thres = 0.02;
    queue<int> terms;
    for (int i = 0; i < dets.size(); i++) {
	if ( abs(dets[i].coeff) > thres ) {
	    slater << "CSF { " << dets[i].coeff*flag << " 1.0 } " << endl;
	    }
	else
            terms.push(i);
	}
    cout << "Total number of Determinants: " << dets.size() - terms.size() << endl;

//    cout << terms.size() << endl;
//    cout << terms.front() << endl;
//    cout << terms.empty() << endl;

    slater << endl;
    slater << "STATES { " << endl;
    int count_det = 0;
    for (int i = 0; i < dets.size(); i++) {
	if ( i != terms.front() or terms.empty() ) {
	    slater << "   " << dets[i].orbs << endl;
	    count_det += 1;
	}
	else 
            terms.pop();
    }
    slater << "}" << endl;

}

void read_ci(ifstream &dirac,ofstream &slater, int &root, double &thres) {
    string line;
    int curr_root = 0;
    vector <det> dets;
    while (dirac.good()) {
	getline(dirac,line);
        if (line.find("Analysis of Optimized") != line.npos) {
	    vector <string> words;
	    parse(line,words);
	    for (int i = 0; i < words.size(); i++) {
		curr_root = StringToNumber<int>(words.back());
	    }
	}
	if (curr_root == root) {
            if (line.find("Coefficient of combination") != line.npos) {
                vector <string> words;
		parse(line,words);
                dets.push_back(det());
		dets.back().coeff = StringToNumber<double>(words.back());
	    }
	    else if (line.find("Corresponding alpha- and beta-strings") != line.npos ) continue;
	    else if (line.find(" alpha ") != line.npos) {
                vector <string> words;
		parse(line,words);
		for (int i = 0; i < words.size(); i++) {
		    if (i%2 != 0) {
			trim_right(words[i]);
			int x = StringToNumber<int>(words[i]);
			x = 2*x-1;
                        dets.back().orbs += " " + NumberToString(x);
		    }
		}
	    }
	    else if (line.find(" beta ") != line.npos) {
		vector <string> words;
		parse(line,words);
		for (int i = 0; i < words.size(); i++) {
		    if (i%2 != 0) {
			trim_right(words[i]);
			int x = StringToNumber<int>(words[i]);
			x *= 2;
			dets.back().orbs += " " + NumberToString(x);
		    }
		}
	    }
	}
    }
    write_dets(slater,dets,thres);
}

void read_ci_c2v(ifstream &dirac,ofstream &slater, int &root, double &thres) {
    string line;
    int curr_root = 0;
    vector <det> dets;
    while (dirac.good()) {
	getline(dirac,line);
        if (line.find("Analysis of Optimized") != line.npos) {
	    vector <string> words;
	    parse(line,words);
	    for (int i = 0; i < words.size(); i++) {
		curr_root = StringToNumber<int>(words.back());
	    }
	}
	if (curr_root == root) {
            if (line.find("Coefficient of combination") != line.npos) {
                vector <string> words;
		parse(line,words);
                dets.push_back(det());
		dets.back().coeff = StringToNumber<double>(words.back());
	    }
	    else if (line.find("Corresponding alpha- and beta-strings") != line.npos ) continue;
	    else if (line.find(" alpha ") != line.npos) {
                vector <string> words;
		parse(line,words);
		for (int i = 1; i < words.size(); i++) {
		    int x = StringToNumber<int>(words[i]);
		    x = 2*x-1;
                    dets.back().orbs += " " + NumberToString(x);
		}
	    }
	    else if (line.find(" beta ") != line.npos) {
		vector <string> words;
		parse(line,words);
		for (int i = 1; i < words.size(); i++) {
		    int x = StringToNumber<int>(words[i]);
		    x *= 2;
		    dets.back().orbs += " " + NumberToString(x);
		}
	    }
	}
    }
    write_dets(slater,dets,thres);
}

int main(int argc, char** argv) {
    
    int err_flag = 0;
    read_input(argc,argv,err_flag);
    if (err_flag != 0) {
	return err_flag;
    }

    ifstream dirac_file;
    dirac_file.open(argv[1]);
    ofstream slater_file;
    string base_name = string(argv[2]);
    string slater_file_name = base_name+".slater";
    slater_file.open(slater_file_name.c_str());
    int root = StringToNumber<int>(string(argv[3]));

    double thres = StringToNumber<double>(string(argv[4]));

    find_nmo(dirac_file,slater_file,base_name);
    dirac_file.clear();
    dirac_file.seekg(0, ios::beg);
#ifdef C2V
    read_ci_c2v(dirac_file,slater_file,root,thres);
#else
    read_ci(dirac_file,slater_file,root,thres);
#endif

    dirac_file.close();
}
