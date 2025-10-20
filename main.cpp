//Logan's Changes Made [Sprint 3]:
//Re-added changes and fixes from previous sprint 2 main
//Modified player structure to include EqID for each session. Not stored into DB.
	//Added prompts to fill in EqID on player addition, broadcasts EqID instead of playerid now.
//[UDP] Sync'd timers

#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QFrame>
#include <QFont>
#include <QTimer>
#include <QPixmap>
#include <QLineEdit>
#include <algorithm>
#include <QKeyEvent>

//Logan's Dependencies
#include "UDPBroadcaster.h"
#include <cstdlib>
#include <string>

//Bryan's Dependencies
#include <QInputDialog>
#include <QMessageBox>
#include <iostream>
#include <string>
#include <postgresql/libpq-fe.h>

//Player Framework
struct Player
{
    int id;
	int EqID;
    QString codename;
    QString status;
};


//Text Helper
QString formatPlayer(const Player& p)
{
    return QString("ID:%1  %2  EqID:%4  - %3").arg(p.id).arg(p.codename).arg(p.status).arg(p.EqID);
}

// --------------------- PostgreSQL helpers ---------------------

// Attempt to read a player (id, codename) by id. Returns true if found.
bool getPlayerFromDB(int id, Player &out) {
    const char* conninfo = "dbname=photon user=student";
    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "DB connect failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return false;
    }

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };
    PGresult* res = PQexecParams(conn,
                                 "SELECT id, codename FROM players WHERE id = $1;",
                                 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        // if there are zero rows, PGRES_TUPLES_OK still, but handle otherwise
        PQclear(res);
        PQfinish(conn);
        return false;
    }

    int rows = PQntuples(res);
    if (rows < 1) {
        PQclear(res);
        PQfinish(conn);
        return false;
    }

    const char* idVal = PQgetvalue(res, 0, 0);
    const char* codenameVal = PQgetvalue(res, 0, 1);

    out.id = std::atoi(idVal);
    out.codename = QString::fromUtf8(codenameVal);
    out.status = "Ready";

    PQclear(res);
    PQfinish(conn);
    return true;
}

// Insert id,codename into players. Returns true on success.
bool insertPlayerToDB(int id, const QString &codename) {
    const char* conninfo = "dbname=photon user=student";
    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "DB connect failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return false;
    }

    std::string idStr = std::to_string(id);
    std::string codeStd = codename.toStdString();
    const char* params[2] = { idStr.c_str(), codeStd.c_str() };

    PGresult* res = PQexecParams(conn,
                                 "INSERT INTO players (id, codename) VALUES ($1::int, $2);",
                                 2, nullptr, params, nullptr, nullptr, 0);

    bool ok = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!ok) {
        std::cerr << "INSERT failed: " << PQerrorMessage(conn) << "\n";
    }

    PQclear(res);
    PQfinish(conn);
    return ok;
}

// Delete player by id. Returns true if a row was deleted.
bool removePlayerFromDBById(int id) {
    const char* conninfo = "dbname=photon user=student";
    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "DB connect failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return false;
    }

    std::string idStr = std::to_string(id);
    const char* params[1] = { idStr.c_str() };

    PGresult* res = PQexecParams(conn,
                                 "DELETE FROM players WHERE id = $1;",
                                 1, nullptr, params, nullptr, nullptr, 0);

    bool ok = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!ok) {
        // deletion failed
        std::cerr << "DELETE failed: " << PQerrorMessage(conn) << "\n";
        PQclear(res);
        PQfinish(conn);
        return false;
    }

    // check how many rows were affected
    char *tuplesAffected = PQcmdTuples(res);
    int affected = 0;
    if (tuplesAffected && tuplesAffected[0] != '\0') affected = std::atoi(tuplesAffected);

    PQclear(res);
    PQfinish(conn);

    return (affected > 0);
}


//Application
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);



    //---------------- Splash Screen ----------------
    QWidget splash;
    splash.setWindowTitle("Laser Tag - Splash Screen");
    splash.setFixedSize(900, 600);
    QVBoxLayout splashLayout(&splash);
    QLabel splashImage;
    splashImage.setAlignment(Qt::AlignCenter);

    //Load Logo Image
    QPixmap pixmap("images/Logo.jpg");
    if (!pixmap.isNull())
    {
        splashImage.setPixmap(pixmap.scaled(splash.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        splashImage.setText("Logo not found");
        splashImage.setStyleSheet("color: white; font-size: 20px;");
    }
    splashLayout.addWidget(&splashImage);

    //---------------- Shared Player Storage ----------------
    QList<Player> teamAPlayers;
    QList<Player> teamBPlayers;

    //---------------- Main Screen ----------------
    //QWidget mainScreen;
	// Create custom widget with key event handling
    class GameWindow : public QWidget {
    public:
        GameWindow(QPushButton* sGameBtn = nullptr, QPushButton* cPlayersBtn = nullptr) 
            : startGameBtn(sGameBtn), clearPlayersBtn(cPlayersBtn) {}
        
        void setButtons(QPushButton* sGameBtn, QPushButton* cPlayersBtn) {
            startGameBtn = sGameBtn;
            clearPlayersBtn = cPlayersBtn;
        }
        
    protected:
        void keyPressEvent(QKeyEvent* event) override {
            if (event->key() == Qt::Key_F5 && startGameBtn) {
                startGameBtn->click();
            }
            else if (event->key() == Qt::Key_F12 && clearPlayersBtn) {
                clearPlayersBtn->click();
            }
            QWidget::keyPressEvent(event);
        }
        
    private:
        QPushButton* startGameBtn;
        QPushButton* clearPlayersBtn;
    };

    GameWindow mainScreen;
    mainScreen.setWindowTitle("Laser Tag - Main Screen");
    mainScreen.setFixedSize(900, 600);
    mainScreen.setStyleSheet("background-color: #1a1525; color: #f0f0f0;");
    QVBoxLayout mainLayout(&mainScreen);


    //---------------- Logo Image ----------------
    QLabel mainLogo;
    QPixmap mainPixmap("images/Logo_Simple.png");
    if (!mainPixmap.isNull())
    {
        mainLogo.setPixmap(mainPixmap.scaled(300, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        mainLogo.setText("Logo_Simple not found");
        mainLogo.setAlignment(Qt::AlignCenter);
        mainLogo.setStyleSheet("color: #f0f0f0; border: 1px solid #4a4f4d;");
        mainLogo.setFixedSize(300, 150);
    }
    mainLogo.setAlignment(Qt::AlignCenter);
    mainLayout.addWidget(&mainLogo, 0, Qt::AlignHCenter);


    //---------------- Upper Divider ----------------
    QFrame logoDivider;
    logoDivider.setFrameShape(QFrame::HLine);
    logoDivider.setFrameShadow(QFrame::Plain);
    logoDivider.setStyleSheet(
        "color: #004a85; "
        "background-color: #004a85; "
        "margin-top: 32px; "
        "margin-bottom: 32px; "
        "height: 3px;"
    );
    mainLayout.addWidget(&logoDivider);


    //---------------- Upper Buttons (Ready, Quit) ----------------
    QHBoxLayout topButtonLayout;

    //Ready Game Button
    QPushButton readyGameBtn("Ready Game");
    topButtonLayout.addWidget(&readyGameBtn);

    //Quit Application Button
    QPushButton quitBtn("Quit Application");
    topButtonLayout.addWidget(&quitBtn);

    mainLayout.addLayout(&topButtonLayout);


    //---------------- Input Line (ID, Add, Remove, Clear) ----------------
    QHBoxLayout addPlayerLayout;

    //ID Input Box
    QLineEdit playerIdInput;
    playerIdInput.setPlaceholderText("Enter Player ID...");
    playerIdInput.setFixedWidth(200);
    addPlayerLayout.addWidget(&playerIdInput);

    //Add Player Button
    QPushButton addPlayerBtn("Add Player");
    addPlayerLayout.addWidget(&addPlayerBtn);

    //Remove Player Button
    QPushButton removePlayerBtn("Remove Player");
    addPlayerLayout.addWidget(&removePlayerBtn);

    //Clear Players Button
    QPushButton clearPlayersBtn("Clear Players");
    addPlayerLayout.addWidget(&clearPlayersBtn);
	
	// Set up keyboard shortcut buttons
    mainScreen.setButtons(&readyGameBtn, &clearPlayersBtn);

    mainLayout.addLayout(&addPlayerLayout);


    //---------------- Middle Divider ----------------
    QFrame middleDivider;
    middleDivider.setFrameShape(QFrame::HLine);
    middleDivider.setFrameShadow(QFrame::Plain);
    middleDivider.setStyleSheet(
        "color: #44125cff;"
        "background-color: #44125cff;"
        "margin-top: 64px;"
        "margin-bottom: 64px;"
        "height: 3px;"
    );
    mainLayout.addWidget(&middleDivider);


    //---------------- Text Boxes ----------------
    QHBoxLayout boxesLayout;
    boxesLayout.setSpacing(12);

    //Status Box
    QFrame statusFrame;
    statusFrame.setFrameShape(QFrame::NoFrame);
    statusFrame.setStyleSheet("QFrame { border: 2px solid #403354; border-radius: 6px; background-color: #2a2139; }");
    statusFrame.setMinimumWidth(300);

    QVBoxLayout statusFrameLayout(&statusFrame);
    statusFrameLayout.setContentsMargins(8,8,8,8);
    statusFrameLayout.setSpacing(4);

    QTextEdit statusBox;
    statusBox.setReadOnly(true);
    statusBox.setFrameShape(QFrame::NoFrame);
    statusBox.setStyleSheet("background: transparent; color: #eaeaea; border: none;");
    statusBox.setText("Game status will appear here...");
    statusFrameLayout.addWidget(&statusBox);
    boxesLayout.addWidget(&statusFrame, 2);


    //Team A (Red) Box
    QFrame teamAFrame;
    teamAFrame.setFrameShape(QFrame::NoFrame);
    teamAFrame.setStyleSheet("QFrame { border: 3px solid #922f34; border-radius: 6px; background-color: #2a2139; }");
    teamAFrame.setMinimumWidth(220);

    QVBoxLayout teamALayout(&teamAFrame);
    teamALayout.setContentsMargins(6,6,6,6);
    teamALayout.setSpacing(4);

    QLabel teamALabel("Team A");
    QFont teamAFont = teamALabel.font();
    teamAFont.setBold(true);
    teamAFont.setUnderline(true);
    teamALabel.setFont(teamAFont);
    teamALabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    teamALabel.setAlignment(Qt::AlignCenter);

    QListWidget teamAList;
    teamAList.setFrameShape(QFrame::NoFrame);
    teamAList.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");

    teamALayout.addWidget(&teamALabel);
    teamALayout.addWidget(&teamAList);


    //Team B (Green) Box
    QFrame teamBFrame;
    teamBFrame.setFrameShape(QFrame::NoFrame);
    teamBFrame.setStyleSheet("QFrame { border: 3px solid #4fb269; border-radius: 6px; background-color: #2a2139; }");
    teamBFrame.setMinimumWidth(220);

    QVBoxLayout teamBLayout(&teamBFrame);
    teamBLayout.setContentsMargins(6,6,6,6);
    teamBLayout.setSpacing(4);

    QLabel teamBLabel("Team B");
    QFont teamBFont = teamBLabel.font();
    teamBFont.setBold(true);
    teamBFont.setUnderline(true);
    teamBLabel.setFont(teamBFont);
    teamBLabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    teamBLabel.setAlignment(Qt::AlignCenter);

    QListWidget teamBList;
    teamBList.setFrameShape(QFrame::NoFrame);
    teamBList.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");

    teamBLayout.addWidget(&teamBLabel);
    teamBLayout.addWidget(&teamBList);

    boxesLayout.addWidget(&teamAFrame, 1);
    boxesLayout.addWidget(&teamBFrame, 1);
    mainLayout.addLayout(&boxesLayout);


    //---------------- Lower Divider ----------------
    QFrame lowerDivider;
    lowerDivider.setFrameShape(QFrame::HLine);
    lowerDivider.setFrameShadow(QFrame::Plain);
    lowerDivider.setStyleSheet(
        "color: #44125cff; "
        "background-color: #44125cff; "
        "margin-top: 64px; "
        "margin-bottom: 64px; "
        "height: 3px;"
    );
    mainLayout.addWidget(&lowerDivider);


    //---------------- Network Buttons ----------------
    QHBoxLayout networkLayout;

    //IP Input Box
    QLineEdit ipInput;
    ipInput.setPlaceholderText("Enter new IP...");
    ipInput.setFixedWidth(200);
    networkLayout.addWidget(&ipInput);

    //Change IP Address Button
    QPushButton changeIpBtn("Change IP Address");
    networkLayout.addWidget(&changeIpBtn);

    //Current IP Text
    QLabel currentIpLabel("Current IP: 127.0.0.1");
    currentIpLabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    currentIpLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    currentIpLabel.setMinimumWidth(200);
    networkLayout.addWidget(&currentIpLabel);
    QString currentIp = "127.0.0.1"; //Default IP setter

    mainLayout.addLayout(&networkLayout);



    //---------------- Game Screen ----------------
    QWidget gameScreen;
    gameScreen.setWindowTitle("Laser Tag - Game Screen");
    gameScreen.setFixedSize(900, 600);
    gameScreen.setStyleSheet("background-color: #1a1525; color: #f0f0f0;");
    QVBoxLayout gameLayout(&gameScreen);


    //---------------- Timer ----------------
    //Upper Text (Status)
    QLabel timerTitleLabel("No Game in Progress");
    timerTitleLabel.setAlignment(Qt::AlignCenter);
    QFont titleFont = timerTitleLabel.font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    timerTitleLabel.setFont(titleFont);
    timerTitleLabel.setStyleSheet("color: #f0f0f0;");
    gameLayout.addWidget(&timerTitleLabel, 0, Qt::AlignHCenter);

    //Lower Text (Value)
    QLabel timerValueLabel("");
    timerValueLabel.setAlignment(Qt::AlignCenter);
    QFont valueFont = timerValueLabel.font();
    valueFont.setPointSize(22);
    valueFont.setBold(true);
    timerValueLabel.setFont(valueFont);
    timerValueLabel.setStyleSheet("color: #f0f0f0;");
    gameLayout.addWidget(&timerValueLabel, 0, Qt::AlignHCenter);


    //---------------- Upper Divider ----------------
    QFrame gameTopDivider;
    gameTopDivider.setFrameShape(QFrame::HLine);
    gameTopDivider.setFrameShadow(QFrame::Plain);
    gameTopDivider.setStyleSheet(
        "color: #004a85;"
        "background-color: #004a85;"
        "margin-top: 32px;"
        "margin-bottom: 32px;"
        "height: 3px;"
    );
    gameLayout.addWidget(&gameTopDivider);


    //---------------- Game Buttons (Start, Pause, End) ----------------
    QHBoxLayout gameButtonLayout;

    //Start Button
    QPushButton gameStartBtn("Start");
    gameButtonLayout.addWidget(&gameStartBtn);

    //Pause Button
    QPushButton gamePauseBtn("Pause Game");
    gameButtonLayout.addWidget(&gamePauseBtn);

    //End Button
    QPushButton gameEndBtn("End Game");
    gameButtonLayout.addWidget(&gameEndBtn);
    gameLayout.addLayout(&gameButtonLayout);


    //---------------- Lower Divider ----------------
    QFrame gameBottomDivider;
    gameBottomDivider.setFrameShape(QFrame::HLine);
    gameBottomDivider.setFrameShadow(QFrame::Plain);
    gameBottomDivider.setStyleSheet(
        "color: #44125cff;"
        "background-color: #44125cff;"
        "margin-top: 64px;"
        "margin-bottom: 64px;"
        "height: 3px;"
    );
    gameLayout.addWidget(&gameBottomDivider);

    
    //---------------- Text Boxes ----------------
    QHBoxLayout gameBoxesLayout;
    gameBoxesLayout.setSpacing(12);

    //Terminal Box
    QFrame gameStatusFrame;
    gameStatusFrame.setFrameShape(QFrame::NoFrame);
    gameStatusFrame.setStyleSheet("QFrame { border: 2px solid #403354; border-radius: 6px; background-color: #2a2139; }");
    gameStatusFrame.setMinimumWidth(300);

    QVBoxLayout gameStatusFrameLayout(&gameStatusFrame);
    gameStatusFrameLayout.setContentsMargins(8,8,8,8);
    gameStatusFrameLayout.setSpacing(4);

    QTextEdit gameStatusBox;
    gameStatusBox.setReadOnly(true);
    gameStatusBox.setFrameShape(QFrame::NoFrame);
    gameStatusBox.setStyleSheet("background: transparent; color: #eaeaea; border: none;");
    gameStatusBox.setText("Game events will appear here...");
    gameStatusFrameLayout.addWidget(&gameStatusBox);
    gameBoxesLayout.addWidget(&gameStatusFrame, 2);


    //Team A (Red) Box
    QFrame gameTeamAFrame;
    gameTeamAFrame.setFrameShape(QFrame::NoFrame);
    gameTeamAFrame.setStyleSheet("QFrame { border: 3px solid #922f34; border-radius: 6px; background-color: #2a2139; }");
    gameTeamAFrame.setMinimumWidth(220);

    QVBoxLayout gameTeamALayout(&gameTeamAFrame);
    gameTeamALayout.setContentsMargins(6,6,6,6);
    gameTeamALayout.setSpacing(4);

    QLabel gameTeamALabel("Team A");
    QFont gameTeamAFont = gameTeamALabel.font();
    gameTeamAFont.setBold(true);
    gameTeamAFont.setUnderline(true);
    gameTeamALabel.setFont(gameTeamAFont);
    gameTeamALabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    gameTeamALabel.setAlignment(Qt::AlignCenter);

    QListWidget gameTeamAList;
    gameTeamAList.setFrameShape(QFrame::NoFrame);
    gameTeamAList.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");

    gameTeamALayout.addWidget(&gameTeamALabel);
    gameTeamALayout.addWidget(&gameTeamAList);


    //Team B (Green) Box
    QFrame gameTeamBFrame;
    gameTeamBFrame.setFrameShape(QFrame::NoFrame);
    gameTeamBFrame.setStyleSheet("QFrame { border: 3px solid #4fb269; border-radius: 6px; background-color: #2a2139; }");
    gameTeamBFrame.setMinimumWidth(220);

    QVBoxLayout gameTeamBLayout(&gameTeamBFrame);
    gameTeamBLayout.setContentsMargins(6,6,6,6);
    gameTeamBLayout.setSpacing(4);

    QLabel gameTeamBLabel("Team B");
    QFont gameTeamBFont = gameTeamBLabel.font();
    gameTeamBFont.setBold(true);
    gameTeamBFont.setUnderline(true);
    gameTeamBLabel.setFont(gameTeamBFont);
    gameTeamBLabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    gameTeamBLabel.setAlignment(Qt::AlignCenter);

    QListWidget gameTeamBList;
    gameTeamBList.setFrameShape(QFrame::NoFrame);
    gameTeamBList.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");

    gameTeamBLayout.addWidget(&gameTeamBLabel);
    gameTeamBLayout.addWidget(&gameTeamBList);

    gameBoxesLayout.addWidget(&gameTeamAFrame, 1);
    gameBoxesLayout.addWidget(&gameTeamBFrame, 1);
    gameLayout.addLayout(&gameBoxesLayout);


    //---------------- Timer Logic ----------------
    enum class GameState { Idle, Cooldown, Running, Paused, Ended };
    GameState gameState = GameState::Idle;

    QTimer tick;
    tick.setInterval(1000);

    int cooldownRemaining = 0;
    int gameRemaining = 0;
    int savedGameRemaining = 0;

    //Update Timer Text
    auto updateTimerLabel = [&]()
    {
        if (gameState == GameState::Idle || gameState == GameState::Ended)
        {
            timerTitleLabel.setText("No Game in Progress");
            timerValueLabel.setText("");
        }
        else if (gameState == GameState::Cooldown)
        {
            timerTitleLabel.setText("Starting In");
            timerValueLabel.setText(QString("%1s").arg(cooldownRemaining));
        }
        else if (gameState == GameState::Running || gameState == GameState::Paused)
        {
            timerTitleLabel.setText("Time Remaining");
            int mins = gameRemaining / 60;
            int secs = gameRemaining % 60;
            timerValueLabel.setText(QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')));
        }
    };

    std::function<void()> endGameRoutine;
    std::function<void()> startCooldownThenRun;

    //Reset Timer on Reset
    endGameRoutine = [&]()
    {
        tick.stop();
        gameState = GameState::Ended;
        for (auto& p : teamAPlayers) p.status = "Ready";
        for (auto& p : teamBPlayers) p.status = "Ready";

        teamAList.clear();
        for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
        teamBList.clear();
        for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

        gameTeamAList.clear();
        for (const auto& p : teamAPlayers) gameTeamAList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
        gameTeamBList.clear();
        for (const auto& p : teamBPlayers) gameTeamBList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));

        updateTimerLabel();
        gameStatusBox.append("Game ended.");
    };

    //Cooldown Timer
    startCooldownThenRun = [&]()
    {
        cooldownRemaining = 5;
        gameState = GameState::Cooldown;
        updateTimerLabel();
        tick.start();
    };

    //Game Timer
    QObject::connect(&tick, &QTimer::timeout, [&]()
    {
        if (gameState == GameState::Cooldown)
        {
            cooldownRemaining--;
            if (cooldownRemaining <= 0)
            {
                gameRemaining = (savedGameRemaining > 0 ? savedGameRemaining : 60);
                gameState = GameState::Running;
                updateTimerLabel();
                gameStatusBox.append("Game started.");
            }
            else
            {
                updateTimerLabel();
            }
        }
        else if (gameState == GameState::Running)
        {
            gameRemaining--;
            if (gameRemaining <= 0)
            {
                endGameRoutine();
            }
            else
            {
                updateTimerLabel();
            }
        }
    });


    //---------------- Main Screen Button Actions ----------------
    //Quit
    QObject::connect(&quitBtn, &QPushButton::clicked, &app, &QApplication::quit);

    //Clear Players
    QObject::connect(&clearPlayersBtn, &QPushButton::clicked, [&]()
    {
        teamAPlayers.clear();
        teamBPlayers.clear();
        teamAList.clear();
        teamBList.clear();
        statusBox.append("Players cleared from both teams.");
    });

    //Add Players: uses DB lookup by ID; if missing, prompt for codename and insert
    QObject::connect(&addPlayerBtn, &QPushButton::clicked, [&]() 
    {
		//Prompt EqID
		bool gotEqID = false;
		int equipmentid = -1;
		QString tempEqID = QInputDialog::getText(&mainScreen, "Enter Equipment ID", QString("Enter Player's Equipment ID:"), QLineEdit::Normal, "", &gotEqID);
		if (!gotEqID || tempEqID.trimmed().isEmpty()) {
			statusBox.append("[Cancelled EqID]");
			return;
		}
		equipmentid = tempEqID.toInt();
		
        bool ok;
        int enteredId = playerIdInput.text().toInt(&ok);
        if (!ok) 
        {
            statusBox.append("Invalid ID entered.");
            return;
        }

        auto alreadyIn = [&](const QList<Player>& team)
        {
            return std::any_of(team.begin(), team.end(), [&](const Player& p){ return p.id == enteredId; });
        };

        if (alreadyIn(teamAPlayers) || alreadyIn(teamBPlayers)) 
        {
            statusBox.append("Player already in game.");
            return;
        }

        Player p;
		p.EqID = equipmentid;
        if (getPlayerFromDB(enteredId, p)) {
            // found in DB: add to appropriate team
            if (enteredId % 2 == 1) 
            {
                teamAPlayers.append(p);
                teamAList.addItem(formatPlayer(p));
                statusBox.append(p.codename + " added to Team A.");
            } else 
            {
                teamBPlayers.append(p);
                teamBList.addItem(formatPlayer(p));
                statusBox.append(p.codename + " added to Team B.");
            }
        } else {
            // not in DB: prompt for codename, then insert
            bool gotName = false;
            QString codename = QInputDialog::getText(&mainScreen,
                                                     "Add Player",
                                                     QString("Player with ID %1 not found in DB.\nEnter codename to create player:").arg(enteredId),
                                                     QLineEdit::Normal,
                                                     "",
                                                     &gotName);
            if (!gotName || codename.trimmed().isEmpty()) {
                statusBox.append("Player with ID " + QString::number(enteredId) + " not found.");
                return;
            }

            if (!insertPlayerToDB(enteredId, codename.trimmed())) {
                statusBox.append("Failed to insert player into database.");
                return;
            }

            // create Player and add to team
            Player newP;
            newP.id = enteredId;
            newP.codename = codename.trimmed();
            newP.status = "Ready";
			newP.EqID = equipmentid;

            if (enteredId % 2 == 1) 
            {
                teamAPlayers.append(newP);
                teamAList.addItem(formatPlayer(newP));
                statusBox.append(newP.codename + " added to Team A.");
            } else 
            {
                teamBPlayers.append(newP);
                teamBList.addItem(formatPlayer(newP));
                statusBox.append(newP.codename + " added to Team B.");
            }
        }
		//Broadcast on add
		const char* NA = qPrintable(currentIp);
		std::string e_ID = std::to_string(equipmentid);
		const char* BID = e_ID.c_str();
		DirectBroadcast(7500, BID, NA);
    });

    //Remove Players: remove from DB by id and from GUI lists
    QObject::connect(&removePlayerBtn, &QPushButton::clicked, [&]() 
    {
        bool ok;
        int enteredId = playerIdInput.text().toInt(&ok);
        if (!ok) {
            statusBox.append("Invalid ID entered.");
            return;
        }

        // Try DB deletion first
        if (!removePlayerFromDBById(enteredId)) {
            statusBox.append("Player with ID " + QString::number(enteredId) + " not found.");
            return;
        }

        // Remove from GUI lists (if present)
        auto removeFromTeam = [&](QList<Player>& team, QListWidget& list, const QString& teamName) 
        {
            for (int i = 0; i < team.size(); ++i) 
            {
                if (team[i].id == enteredId) {
                    statusBox.append(team[i].codename + " removed from " + teamName + ".");
                    team.removeAt(i);
                    list.clear();
                    for (const auto& p : team) list.addItem(formatPlayer(p));
                    return true;
                }
            }
            return false;
        };

        if (!removeFromTeam(teamAPlayers, teamAList, "Team A") &&
            !removeFromTeam(teamBPlayers, teamBList, "Team B")) 
        {
            // player was deleted from DB but wasn't in current GUI lists
            statusBox.append("Player with ID " + QString::number(enteredId) + " removed from database.");
        }

        playerIdInput.clear();
    });

    //Change IP
    QObject::connect(&changeIpBtn, &QPushButton::clicked, [&]()
    {
        QString newIp = ipInput.text();
        if (!newIp.isEmpty())
        {
            currentIp = newIp;
            currentIpLabel.setText("Current IP: " + currentIp);
            statusBox.append("Network IP changed to: " + currentIp);
        }
        else
        {
            statusBox.append("No IP entered. Current IP remains " + currentIp);
        }
    });

    //Ready Game
    QObject::connect(&readyGameBtn, &QPushButton::clicked, [&]()
    {
        gameTeamAList.clear();
        for (const auto& p : teamAPlayers) gameTeamAList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
        gameTeamBList.clear();
        for (const auto& p : teamBPlayers) gameTeamBList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));

        teamAList.clear();
        for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
        teamBList.clear();
        for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

        gameScreen.show();
        mainScreen.hide();

        tick.stop();
        gameState = GameState::Idle;
        cooldownRemaining = 0;
        gameRemaining = 0;
        updateTimerLabel();
        gameStatusBox.append("Entered Game Screen");
    });


    //---------------- Game Screen Button Actions ----------------
    //Game Start / Restart
    QObject::connect(&gameStartBtn, &QPushButton::clicked, [&]()
    {
        if (gameStartBtn.text() == "Start")
        {
            if (gameState == GameState::Running || gameState == GameState::Cooldown)
            {
                gameStatusBox.append("Game is already running or in countdown.");
                return;
            }

            for (auto& p : teamAPlayers) p.status = "Active";
            for (auto& p : teamBPlayers) p.status = "Active";

            gameTeamAList.clear();
            for (const auto& p : teamAPlayers) gameTeamAList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
            gameTeamBList.clear();
            for (const auto& p : teamBPlayers) gameTeamBList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));

            teamAList.clear();
            for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
            teamBList.clear();
            for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

            startCooldownThenRun();
            gameStatusBox.append("Game starting.");
            gameStartBtn.setText("Restart Game");
        }
        else
        {
            for (auto& p : teamAPlayers) p.status = "Active";
            for (auto& p : teamBPlayers) p.status = "Active";

            gameTeamAList.clear();
            for (const auto& p : teamAPlayers) gameTeamAList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
            gameTeamBList.clear();
            for (const auto& p : teamBPlayers) gameTeamBList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));

            teamAList.clear();
            for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
            teamBList.clear();
            for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

            startCooldownThenRun();
            gameStatusBox.append("Game restarted.");
            gameStartBtn.setText("Restart Game");
        }
		
		//Logan Edit #1
		const char* NA = qPrintable(currentIp);
		std::string command1 = std::string("xterm -hold -e './Game.exe 1 ") + NA + "' &";
		int sys_check_start = system(command1.c_str());
		std::string command2 = std::string("xterm -hold -e './Game.exe 2 ") + NA + "' &";
		int sys_check_start2 = system(command2.c_str());
    });


    //Game Pause / Unpause
    QObject::connect(&gamePauseBtn, &QPushButton::clicked, [&]()
    {
        if (gameState == GameState::Running)
        {
            tick.stop();
            savedGameRemaining = gameRemaining;
            gameState = GameState::Paused;
            gamePauseBtn.setText("Unpause Game");
            gameStatusBox.append("Game paused.");
        }
        else if (gameState == GameState::Paused)
        {
            gameState = GameState::Running;
            gameRemaining = savedGameRemaining;
            gamePauseBtn.setText("Pause Game");
            tick.start();
            gameStatusBox.append("Game unpaused.");
        }
        else
        {
            gameStatusBox.append("Game not running; Press Start Game to begin.");
        }
    });


    //End Game
    QObject::connect(&gameEndBtn, &QPushButton::clicked, [&]()
    {
        tick.stop();
        gameState = GameState::Ended;

        for (auto& p : teamAPlayers) p.status = "Ready";
        for (auto& p : teamBPlayers) p.status = "Ready";

        teamAList.clear();
        for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
        teamBList.clear();
        for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

        gameTeamAList.clear();
        for (const auto& p : teamAPlayers) gameTeamAList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
        gameTeamBList.clear();
        for (const auto& p : teamBPlayers) gameTeamBList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));

        updateTimerLabel();
        gameScreen.hide();
        mainScreen.show();

        gameStartBtn.setText("Start");
        gamePauseBtn.setText("Pause");
        gameStatusBox.append("Game ended and returning to main screen.");
		
		//Logan Edit #2
		//Closes Traffic Gen & Receiver
		const char* NA = qPrintable(currentIp);
		Broadcast("221", NA);
		Broadcast("221", NA);
		Broadcast("221", NA);
		DirectBroadcast (7501, "303", NA);
		DirectBroadcast (7501, "303", NA);
		DirectBroadcast (7501, "303", NA);
    });


    //Player Event Test (Double-Click Player)
    QObject::connect(&gameTeamAList, &QListWidget::itemDoubleClicked, [&](QListWidgetItem* item)
    {
        QString text = item->text();
        QString name = text.section(" - ", 0, 0);

        for (auto& p : teamAPlayers)
        {
            if (p.codename == name)
            {
                if (p.status == "Active")
                {
                    p.status = "Tagged";
                    gameStatusBox.append(p.codename + " tagged.");
                }
                else
                {
                    p.status = "Active";
                    gameStatusBox.append(p.codename + " reset to Active.");
                }
                break;
            }
        }

        gameTeamAList.clear();
        for (const auto& p : teamAPlayers) gameTeamAList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
    });

    QObject::connect(&gameTeamBList, &QListWidget::itemDoubleClicked, [&](QListWidgetItem* item)
    {
        QString text = item->text();
        QString name = text.section(" - ", 0, 0);
        for (auto& p : teamBPlayers)
        {
            if (p.codename == name)
            {
                if (p.status == "Active")
                {
                    p.status = "Tagged";
                    gameStatusBox.append(p.codename + " tagged.");
                }
                else
                {
                    p.status = "Active";
                    gameStatusBox.append(p.codename + " reset to Active.");
                }
                break;
            }
        } 
        gameTeamBList.clear();
        for (const auto& p : teamBPlayers) gameTeamBList.addItem(QString("%1 - %2").arg(p.codename).arg(p.status));
    });


    //---------------- Splash to Main Switch ----------------
    QTimer::singleShot(4000, [&]()
    {
        splash.close();
        mainScreen.show();
        statusBox.append("Entered main screen.");
        gameState = GameState::Idle;
        updateTimerLabel();
    });

    splash.show();
    return app.exec();
}