#include <QtSql/QtSql>
#include <QDebug>
#include <iostream>

#define DATA_SIZE 512

using namespace std;

string create_win(string title);
void  find_host(string s);

int main(int argc, char *argv[])
{
    QString mask;

    if (argc == 2)
        mask = argv[1];
    else
    {
        cerr << "Usage: trun SEARCH_STRING \n";
        exit(1);
    }
//    qDebug() << mask;
    string domain, user, root, ip, port;

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("wst.loc");
    db.setDatabaseName("passdb");
    db.setUserName("pak");
    db.setPassword("SqlWord");
    bool ok = db.open();
    //    if (ok)
    //        qDebug() << "good open";
    QSqlQuery query;
    QString sel = "SELECT domain,pak,root,ip,port FROM hosts WHERE domain LIKE '%" + mask + "%'";
    query.exec(sel);
    if (query.next())
    {
        domain = query.value(0).toString().toStdString();
        user = query.value(1).toString().toStdString();
        root = query.value(2).toString().toStdString();
        ip = query.value(3).toString().toStdString();
        port = query.value(4).toString().toStdString();
    }

    string p_id = create_win(domain);
    string run_cmd = "tmux send-keys -t " + p_id + " 'ssh " + ip + "\n'";
    system(run_cmd.c_str());
    sleep(1);
    run_cmd = "tmux send-keys -t " + p_id + " '" + user + "\n'";
    system(run_cmd.c_str());
    sleep(1);
    run_cmd = "tmux send-keys -t " + p_id + " 'su\n'";
    system(run_cmd.c_str());
    sleep(1);
    run_cmd = "tmux send-keys -t " + p_id + " '" + root + "\n'";
    system(run_cmd.c_str());
    sleep(1);

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
    //    cout << pane_id << "ggg";

    pclose(pf);

    return pane_id;
}
