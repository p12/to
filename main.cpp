#include <QtSql/QtSql>
#include <QDebug>
#include <iostream>
#include <cstdio>

#define DATA_SIZE 512

using namespace std;

string create_win(string title);
string waitPass(string pid);


int main(int argc, char *argv[])
{
    QString mask, userName, dbPass, host, type;

    int rez=0;
    while ( (rez = getopt(argc,argv,"u:h:s:t:p:")) != -1)
        switch (rez){
        case 'u':
            userName = optarg;      //username
            break;
        case 's':
            mask = optarg;          //search mask
            break;
        case 'h':
            host = optarg;          //database host
            break;
        case 't':
            type = optarg;          //connection type
            break;
        case 'p':
            dbPass = optarg;        //database password
            break;
        };

    string domain, user, root, ip, port, enc, pass;

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setDatabaseName("passdb");
    db.setUserName(userName);
    db.setPassword(dbPass);
    db.setConnectOptions("CLIENT_SSL=1");
    if (!db.open() )
    {
        cerr << "Unable connect to database.\n";
        exit(1);
    }

    QSqlQuery query;

    if (type == "ssh")
    {
        QString slct = "SELECT domain," + userName +
                ",root,ip,port,enc FROM hosts WHERE (domain LIKE '%"
                + mask + "%') or (ip LIKE '%" + mask + "')";
        query.exec(slct);
        if (query.next())
        {
            domain = query.value(0).toString().toStdString();   //hostname
            user = query.value(1).toString().toStdString();     //user password
            root = query.value(2).toString().toStdString();     //root password
            ip   = query.value(3).toString().toStdString();     //ip address
            port = query.value(4).toString().toStdString();     //ssh port
            enc  = query.value(5).toString().toStdString();     //server encoding

            string p_id = create_win(mask.toStdString() );
            string pref = "tmux send-keys -t " + p_id + " -- ";
            string cmd = pref + " 'luit -encoding " + enc + " ssh -p " + port + " " + ip + "\n'";
            system(cmd.c_str());
            while (waitPass(p_id) != "1")
            {
                cout << waitPass(p_id) << endl;
                sleep(1);
            }
            cmd = pref + " '" + user + "\n'";
            system(cmd.c_str());
            sleep(1);
            cmd = pref + " 'su\n'";
            system(cmd.c_str());
            sleep(1);
            cmd = pref + " '" + root + "\n'";
            system(cmd.c_str());
        }
    } else if (type == "telnet")
    {
        QString slct = "SELECT ip,username,password FROM switches WHERE ip LIKE '%." + mask + "'";
        query.exec(slct);
        if (query.next())
        {
            ip    = query.value(0).toString().toStdString();     //ip address
            user  = query.value(1).toString().toStdString();     //username
            pass  = query.value(2).toString().toStdString();     //password

            string p_id = create_win(mask.toStdString() );
            string pref = "tmux send-keys -t " + p_id + " -- ";
            string cmd = pref + " 'telnet " + ip + "\n'";
            system(cmd.c_str());
            sleep(1);
            cmd = pref + " '" + user + "' C-m";
            system(cmd.c_str());
            sleep(1);
            cmd = pref + " '" + pass + "' C-m";
            system(cmd.c_str());
        }
    }

    return 0;
}

string create_win(string title)
{
    FILE *pf;
    string cmd = "tmux neww -P -n " + title;
    char buf[DATA_SIZE];

    // Setup our pipe for reading and execute our command.
    pf = popen(cmd.c_str(),"r");

    if(!pf){
        cerr << "Could not open pipe for output.\n";
        exit(1);
    }

    // Grab data from process execution
    fgets(buf, DATA_SIZE , pf);

    string pane_id(buf);
    pane_id.erase(pane_id.length() - 1);

    pclose(pf);

    return pane_id;
}

string waitPass(string pid)
{
    FILE *pf;
    string cmd = "tmux  capturep -p -S -20 -t " + pid + " | grep -c -i password\n";
    char buf[DATA_SIZE];

    // Setup our pipe for reading and execute our command.
    pf = popen(cmd.c_str(),"r");

    if(!pf){
        cerr << "Could not open pipe for output.\n";
        exit(1);
    }

    // Grab data from process execution
    fgets(buf, DATA_SIZE , pf);

    string passCnt(buf);
    passCnt.erase(passCnt.length() - 1);

    pclose(pf);

    return passCnt;
}
