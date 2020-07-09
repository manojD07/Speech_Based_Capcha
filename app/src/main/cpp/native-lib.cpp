#include <jni.h>
//#include <strings.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <iomanip>


using namespace std;

std::string path ="/storage/emulated/0/Android/data/com.example.native_speech_with_c/files/";

/*define macros for N=No. of states, M = no. of distinct obs. symbol per state,T = time at which we want to calc. prob*/
#define N 5
#define T 80
#define M 32
#define O 30
/////////////////////////////////////// Global Variables Declaration ////////////////////////////////////////////////////
long double pi[N];//Initial state distribution vector
long double A[N][N];//State transition probability matrix
long double B[N][M];//observation symbol prob. distribution
long double data[100000],E[200];//Array storing data and energy framewise
int size=0;//size of data array initialization
int ob_count=0;
long double codebook[32][12];//codebook Array
long double maxi=0,shift=0;
int tempFrame[320];//temporary frame
long double hamming[320];//array for storing hamming values
long double ctemp[25][13];
int ctempindex=0;
long double r[13];
long double alpha[T][N];//Forward variable
long double beta[T][N];//backward variable
long double zhi[T][N][N];//Zhita variable
long double gama[T][N];//gamma variable
int obs[O][T];
int p=12;
long double delta[T][N];//delta variable
long double shi[T][N];//shi variable
int state_seq[T];//state sequence
long double avg_A[N][N];//Average State transition probability matrix
long double avg_B[N][M];//Average observation symbol prob. distribution
long double prev_A[N][N];
long double prev_B[N][M];
long double a[13];
long double c[13];
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////Normalization Module//////////////////////////////////////////////////////////////

void normalize(double d)
{
    for(int i=0;i<size;i++)
    {
        data[i] = ((data[i]-d)/maxi)*5000;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////Energy Calculating module /////////////////////////////////////////////////////////////
long double energy(int beg , int end)
{
    long double temp=0;
    for(int i=beg;i<=end;i++)
        temp = temp + data[i]*data[i];
    temp = (temp/(end-beg+1));

    return temp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////  Calculating Cepstral coefficient ////////////////////////////////////////////////////////////////
void cepstralcoefficients()
{
    unsigned int m,j;


    c[0]= logl(r[0]);


    for(m=1;m<=12;m++)
    {
        long double sum=0;
        for(j=1;j<=m-1;j++)
            sum+=j*c[j]*a[m-j]/m;
        c[m]=a[m]+sum;
    }


    m=1;
    long double data;
    ifstream f;
    f.open(path+"raise_sine.txt");
    int i=0;
    while(!f.eof())
    {
        f>>data;
        c[m++]*=data;

    }

    f.close();


}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//****************************************************************DURBIN***********************************************************************************


void durbin()
{
    long double E[13];
    long double K[13];
    long double ar[13][13];
    long double sum=0;
    E[0]=r[0];

    for(int i=1;i<=p;i++)
    {
        sum=0;

        for(int j=1;j<=i-1;j++)
            sum+=(ar[j][i-1]*r[i-j]);

        if(i==1)
            sum=0;

        K[i]=(r[i]-sum)/E[i-1];
        ar[i][i]=K[i];

        for(int j=1;j<=i-1;j++)
        {
            ar[j][i]=ar[j][i-1]-(K[i]*ar[i-j][i-1]);

        }

        E[i]=(1-(K[i]*K[i]))*E[i-1];


    }

    for(int i=1;i<=p;i++)
    {
        a[i]=ar[i][p];

    }

}

/////////////////////////////////////////////////////////// Apply Hamming Window //////////////////////////////////////////////////////////////
void hammingcode()
{ int i=0;
    ifstream f;
    ofstream fo;
    f.open(path+"hamming.txt");
    fo.open(path+"hammingRead.txt");
    while(!f.eof())
    {
        f>>hamming[i];
        fo<<hamming[i]<<endl;
        i++;
    }
    f.close();
    fo.close();
    for(int j=0;j<320;j++)
        tempFrame[j]*=hamming[j];
}




//***************************************************************************************************************************************************************



//****************************************     RI's CALCULATE FUNCTION      *************************************************************************************//
void cal_r()
{

    for(int i=0;i<13;++i)
    {
        r[i]=0;
    }


    for(int j=0;j<13;++j)
    {
        for(int i=0;i<320;++i)
        {
            if(i+j<320)
                r[j]+=tempFrame[i]*tempFrame[(i+j)];

        }
    }


}
//////////////////////////////////////////////////////// Testing Module ///////////////////////////////////////////////////////////////////
void tests(string fname)
{
    size=0;
    int beg=0,end=48000;
    int s;
    long double n;

    ifstream fin;
    fin.open(path + fname);
    if(!fin)
    {
        cout<<"Error openning file";

    }

    while(!fin.eof())
    {
        fin>>n;
        data[size++]=n;
        if(abs(n)>maxi)
            maxi = abs(n);
    }
    fin.close();
    int sum=0;
    int p=0;
    fin.open(path+"silence.txt");
    while(!fin.eof())
    {
        fin>>n;
        sum+=n;
        p++;
    }
    fin.close();

    double shift = (double)sum/p;   // calculating DC shift using ambient noise


    int framesize=100,framecount=460;
    int flag=0;
    for(int i=0;i<framecount;++i)
    {

        int count=0;
        if(flag==1)
            break;

        for(int j=0;j<framesize;++j)
        {

            if(abs((int)data[i*framesize+j])>1000)
            {count++;

            }

        }

        if(count>70)
        {beg=(i-8);
            flag=1;}
    }

    flag=0;

    for(int i=framecount-1;i>beg;--i)
    {
        int count1=0;

        if(flag==1)
            break;

        for(int j=0;j<framesize;++j)
        {
            if(abs((int)data[i*framesize+j])>1000)
            {
                count1++;


            }

        }
        if(count1>50)
        {end=(i+15);
            flag=1;}
    }
    end*=100;
    beg*=100;
    ofstream fo;
    fo.open(path+"trimmed.txt");
    for(int i=beg;i<end;i++)
    {
        fo<<data[i]<<endl;
    }
    fo.close();
    normalize(shift);

    //long double enrgy1;
    //int i=0;
    //while((i+1)*100<size)    //calculating Energy and ZCR for every frame of 320 size
    //{
    //
    //	enrgy1 = energy(i*100,(i*100)+99);
    //	E[i] = enrgy1;
    //
    //i++;
    //}
    //long double avg_Energy=0;
    //for(int s=0;s<i;s++)  // Calculating Average Energy
    //{
    //	avg_Energy += E[s];
    //
    //}
    //avg_Energy  /= i;
    //
    //int flags=1,flage=1;
    //for(int s=0;s<i;s++)  // Calculating start and end marker
    //{
    //	if((E[s] > 1.05*avg_Energy)&&(flags))
    //	{
    //		beg=s*100;
    //		flags=0;
    //	}
    //	if((E[i-s]>.05*avg_Energy)&&(flage))
    //	{
    //		end = (i-s)*100;
    //		flage=0;
    //	}

    //
    //}


    fin.open(path+"codebook.txt");
    fo.open(path+"codebookRead.txt");
    for(int i=0;i<32;i++)
    {
        for(int j=0;j<12;j++)
        {   codebook[i][j]=0;
            fin>>n;

            codebook[i][j]=n;

            fo<<codebook[i][j]<<endl;
        }
    }

    fo.close();
    fin.close();

    for(int i=0;i<13;++i)
    {
        r[i]=0;
    }
    int countt=0;
    while(beg<end)
    {

        for(int i=0;i<320;i++)
        {
            tempFrame[i] = data[beg+i];
        }
        hammingcode();
        cal_r();
        durbin();
        cepstralcoefficients();
        beg = beg+80;

        long double tok_weight[12] ={1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0,63};

        long double temp;
        long double min=INT_MAX;
        long double dist=0;
        int index;

        for(int j=0;j<32;j++)
        {
            dist=0;
            for(int i=0;i<=11;i++)
            {

                dist+= tok_weight[i]*(c[i+1]-codebook[j][i])*(c[i+1]-codebook[j][i]);

            }
            //cout<<"distance from"<<j<<"=="<<dist<<endl;
            if(dist<min)
            {
                min = dist;
                index = j;

            }
        }
        obs[ob_count][countt++] = index+1;
    }
    //cout<<countt;
    ob_count++;
}

//////////////////////////////////////////////////////////Testing Module Digit wise///////////////////////////////////////////////////////////////
/*
void test(string fname)
{
    string temp=fname;

    for(int i=1;i<29;i++)
    {

        if(i!=1)
            fname.append(temp);

        fname.append(to_string((long double)i));
        fname.append(".txt");
        //cout<<fname;
        tests(fname);
        //cout<<endl<<endl;
        fname.clear();
    }

}
*/
//////////////////////////////////////////////////////// Reading File that conatins Models///////////////////////////////////////////////////////////////////
void fread(string fname)
{



    string skip;//string to skip lines in Initial model file
    long double temp;//temporary variable to store long double values from file
    ifstream fin;
    fin.open(path+fname);//open file
    ofstream fo;
    fo.open(path+"sa.txt");
    fo<<path+fname;
    if(!fin)
        cout<<"can't open file";
    else
    {
        while(!fin.eof())
        {

            getline(fin,skip);//skip first 3 lines
            for(int i=0;i<N;i++)
            {
                fin>>temp;
                pi[i]= temp;//reading PI matrix values
                //	cout<<pi[i]<<" ";
                fo<<pi[i]<<"\t";
            }
            getline(fin,skip);
            getline(fin,skip);//skipping 3 lines

            fo<<endl<<endl;
            for(int i=0;i<N;i++)
            {
                for(int j=0;j<N;j++)
                {
                    fin>>temp;//reading the values of A matrix
                    A[i][j] = temp;
                    fo<<A[i][j]<<"\t";
                }
                fo<<endl;
            }
            getline(fin,skip);
            getline(fin,skip);
            fo<<endl<<endl;
            getline(fin,skip);//skipping 3 lines
            for(int i=0;i<N;i++)
            {
                for(int j=0;j<M;j++)
                {
                    fin>>temp;
                    B[i][j] = temp;//reading the values of B matrix
                    fo<<B[i][j]<<"\t";
                }
                fo<<endl;
            }




            break;
        }


    }
    fin.close();
    fo.close();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    string record;
        ob_count=0;

       record = "recording.txt";
        tests(record);
        record.erase();
        ////////////////////////// Initialization////////////////////////////////
        for(int j=0;j<1;j++)
        {
            long double maxx_final= -1;int indexx = -1;
            for(int s=1;s<19;s++)
            {
                string fname;
                fname.append("Lamabda__");
                fname.append(to_string(s));
                fname.append(".txt");
                fread(fname);

                fname.clear();
                for(int i=0;i<N;i++)
                {
                    alpha[0][i] = pi[i]*B[i][obs[j][0]];
                }
                ///////////////////////////////////////////////////////////////////////////

                /////////////////////////// Induction ///////////////////////////////////
                for(int i=0;i<T-1;i++)
                {

                    for(int k=0;k<N;k++)
                    {
                        long double temp = 0;
                        for(int l=0;l<N;l++)
                        {
                            temp += alpha[i][l]*A[l][k];
                        }
                        alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                    }
                }
                ////////////////////////////////////////////////////////////////////////

                //cout<<"Probability for Observation sequence "<<j<<" == ";
                long double fin_prob=0;

                ////////////////////// Termination /////////////////////////////////////
                for(int i=0;i<N;i++)
                {
                    fin_prob +=alpha[T-1][i];
                }
                //cout<<"lambada"<< s<<" prob ="<< fin_prob;
                if(fin_prob>maxx_final)
                {
                    maxx_final = fin_prob;
                    indexx = s;
                }
                ////////////////////////////////////////////////////////////////////////
            }
           // cout<<maxx_final;

            string ans= "Coudnt Recognize";
            if(indexx==1)
                ans = "alexa";

            if(indexx==2)
                ans = "cow";
            if(indexx==3)
                ans = "down";
            if(indexx==4)
                ans = "google";
            if(indexx==5)
                ans = "hello";


            if(indexx==6)
                ans = "risk";

            if(indexx==7)
                ans = "shut";


            if(indexx==8)
                ans = "tiger";



            if(indexx==9)
                ans = "0";

            if(indexx==10)
                ans = "1";
            if(indexx==11)
                ans = "2";
            if(indexx==12)
                ans = "3";
            if(indexx==13)
                ans = "4";


            if(indexx==14)
                ans = "5";

            if(indexx==15)
                ans = "6";


            if(indexx==16)
                ans = "7";


            if(indexx==17)
                ans = "8";

            if(indexx==18)
                ans = "9";

            return env->NewStringUTF(ans.c_str());



    }
  //  Read_CodebookValues();
   // getWindowValues();
    //string hello = "Hello from C++";
   // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
   // return Codebook[0][0];

}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_pcmtotexxt(JNIEnv *env, jobject /*thiz*/) {
    std::ifstream f_in;
    std::ofstream f_out;
    short speech;

    f_in.open("/storage/emulated/0/Android/data/com.example.native_speech_with_c/files/recording.pcm",
              std::ios::in | std::ios::binary);
    f_out.open(
            "/storage/emulated/0/Android/data/com.example.native_speech_with_c/files/recording.txt");

    while (!f_in.eof()) {
        f_in.read((char *) &speech, sizeof(short));

        f_out << speech << std::endl;

    }
    f_in.close();
    f_out.close();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group1(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=1;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "google";




        if(indexx==2)
            ans = "Two";


        if(indexx==3)
            ans = "Seven";




        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}


extern "C" JNIEXPORT jstring JNICALL

Java_com_example_native_1speech_1with_1c_MainActivity_group2(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=2;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "down";




        if(indexx==2)
            ans = "Zero";


        if(indexx==3)
            ans = "Nine";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group3(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=3;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "alexa";




        if(indexx==2)
            ans = "cow";


        if(indexx==3)
            ans = "Six";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}



extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group4(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=4;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "google";




        if(indexx==2)
            ans = "Six";


        if(indexx==3)
            ans = "Seven";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group5(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=5;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "Four";




        if(indexx==2)
            ans = "Five";


        if(indexx==3)
            ans = "hello";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group6(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=6;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "risk";




        if(indexx==2)
            ans = "hello";


        if(indexx==3)
            ans = "Nine";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group7(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=7;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "risk";




        if(indexx==2)
            ans = "cow";


        if(indexx==3)
            ans = "google";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group8(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=8;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "alexa";




        if(indexx==2)
            ans = "Four";


        if(indexx==3)
            ans = "Five";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_native_1speech_1with_1c_MainActivity_group9(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=9;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";

        if(indexx==1)
            ans = "cow";




        if(indexx==2)
            ans = "One";


        if(indexx==3)
            ans = "Eight";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}

extern "C" JNIEXPORT jstring JNICALL

Java_com_example_native_1speech_1with_1c_MainActivity_group10(
        JNIEnv* env,
        jobject /* this */) {

    string record;
    ob_count=0;
    int group=10;
    record = "recording.txt";
    tests(record);
    record.erase();
    ////////////////////////// Initialization////////////////////////////////
    for(int j=0;j<1;j++)
    {
        long double maxx_final= -1;int indexx = 1;
        for(int s=1;s<4;s++)
        {
            string fname;
            fname.append("Lamabda__");
            fname.append(to_string(group));
            fname.append("_");
            fname.append(to_string(s));
            fname.append(".txt");
            fread(fname);

            fname.clear();
            for(int i=0;i<N;i++)
            {
                alpha[0][i] = pi[i]*B[i][obs[j][0]];
            }
            ///////////////////////////////////////////////////////////////////////////

            /////////////////////////// Induction ///////////////////////////////////
            for(int i=0;i<T-1;i++)
            {

                for(int k=0;k<N;k++)
                {
                    long double temp = 0;
                    for(int l=0;l<N;l++)
                    {
                        temp += alpha[i][l]*A[l][k];
                    }
                    alpha[i+1][k] = temp*B[k][obs[j][i+1]];
                }
            }
            ////////////////////////////////////////////////////////////////////////

            //cout<<"Probability for Observation sequence "<<j<<" == ";
            long double fin_prob=0;

            ////////////////////// Termination /////////////////////////////////////
            for(int i=0;i<N;i++)
            {
                fin_prob +=alpha[T-1][i];
            }
            //cout<<"lambada"<< s<<" prob ="<< fin_prob;
            if(fin_prob>maxx_final)
            {
                maxx_final = fin_prob;
                indexx = s;
            }
            ////////////////////////////////////////////////////////////////////////
        }
        // cout<<maxx_final;

        string ans= "Coudnt Recognize";
        if(indexx==1)
            ans = "Three";




        if(indexx==2)
            ans = "Six";


        if(indexx==3)
            ans = "hello";

        return env->NewStringUTF(ans.c_str());



    }
    //  Read_CodebookValues();
    // getWindowValues();
    //string hello = "Hello from C++";
    // hello=hello+"x";
    //return env->NewStringUTF(hello.c_str());
    // return Codebook[0][0];

}



