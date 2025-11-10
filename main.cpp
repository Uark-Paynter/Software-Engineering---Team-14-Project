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
#include "UDPBroadcaster.h"
#include "Scorecard.h"
#include <cstdlib>
#include <string>
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
	int score = 0;
    QString codename;
    QString status;
};


//Text Helper
QString formatPlayer(const Player& p)
{
    return QString("ID:%1  %2  EqID:%4  - %3").arg(p.id).arg(p.codename).arg(p.status).arg(p.EqID);
}

// --------------------- Shared Memory Functions ---------------------

//Receiver Timeout Disable
std::string timeoutcheck = "true";

//Player Score Sync
void syncScores(QList<Player>& teamA, QList<Player>& teamB, Scorecard* scorecard) {
	sem_wait (&scorecard->mutex);
	for(int i = 0; i < scorecard->playercount; i++) {
		int eqID = scorecard->activeIDS[i];
		int storescore = scorecard->scores[i];
		
		for(auto& p : teamA) {
			if(p.EqID == eqID) {
				p.score = storescore;
				break;
			}
		}
		for(auto& p : teamB) {
			if(p.EqID == eqID) {
				p.score = storescore;
				break;
			}
		}
	}
	sem_post(&scorecard->mutex);
}

//Event Handler
void eventHandler(Scorecard* scorecard, QTextEdit* gameStatusBox, const QList<Player>& teamAPlayers, const QList<Player>& teamBPlayers) {
	char buffer[16];
	while(pullEvent(scorecard, buffer, sizeof(buffer))) {
		//Pull IDS
		char breaker  = ':';
		int breaknum = 0;
		char hit[3] = "";
		char tagger[3] = "";
		//Find ":"
		for (int i = 0; buffer[i] != '\0'; i++) {
			if (buffer[i] == breaker) {breaknum = i;}
		}
		//Break Hit/Tag
		for (int i = 0; i < breaknum; i++) {tagger[i] = buffer[i];}
		for (int i = breaknum+1; buffer[i] != '\0'; i++) {hit[i-breaknum-1] = buffer[i];}
		
		//Pull Codenames
		QString taggername = QString("Player %1").arg(atoi(tagger));
		QString hitname = QString("Player %1").arg(atoi(hit));
		auto pullCodename = [&](int eqID) -> QString {
			for (const auto& p : teamAPlayers)
				if (p.EqID == eqID) return p.codename;
			for (const auto& p : teamBPlayers)
				if (p.EqID == eqID) return p.codename;
			return QString("Player %1").arg(eqID);
		};
		taggername = pullCodename(atoi(tagger));
		hitname = pullCodename(atoi(hit));
		
		//Form Output
		QString output;
		if(strcmp(hit, "43") == 0)
			output = QString("%1 tagged the Red Base").arg(taggername);
		else if (strcmp(hit, "53") == 0)
			output = QString("%1 tagged the Green Base").arg(taggername);
		else
			output = QString("%1 tagged %2").arg(taggername).arg(hitname);
		
		//Display Output
		gameStatusBox->append(output);
	}
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
	Scorecard* scorecard = initSharedMemory(true);




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
	
	//Disable Receiver Timeout
	QPushButton allowTimeoutBtn("Disable Receiver Timeout");
	networkLayout.addWidget(&allowTimeoutBtn);

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


    //Game Screen Team Box Setup
    auto createTeamBox = [&](const QString& teamName, const QString& borderColor, QFrame*& frame, QLabel*& totalLabel, QListWidget*& nameList, QListWidget*& scoreList)
    {
        frame = new QFrame();
        frame->setFrameShape(QFrame::NoFrame);
        frame->setStyleSheet(QString("QFrame { border: 3px solid %1; border-radius: 6px; background-color: #2a2139; }").arg(borderColor));
        frame->setMinimumWidth(220);

        QVBoxLayout* mainLayout = new QVBoxLayout(frame);
        mainLayout->setContentsMargins(6,6,6,6);
        mainLayout->setSpacing(4);

        QLabel* label = new QLabel(teamName);
        QFont font = label->font();
        font.setBold(true);
        font.setUnderline(true);
        label->setFont(font);
        label->setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
        label->setAlignment(Qt::AlignCenter);

        totalLabel = new QLabel("Score: 0");
        totalLabel->setAlignment(Qt::AlignCenter);
        totalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");

        QHBoxLayout* teamInnerLayout = new QHBoxLayout();
        nameList = new QListWidget();
        scoreList = new QListWidget();

        nameList->setFrameShape(QFrame::NoFrame);
        scoreList->setFrameShape(QFrame::NoFrame);
        nameList->setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
        scoreList->setStyleSheet("background: transparent; border: none; color: #f0f0f0; text-align: right;");

        teamInnerLayout->addWidget(nameList);
        teamInnerLayout->addWidget(scoreList);

        mainLayout->addWidget(label);
        mainLayout->addWidget(totalLabel);
        mainLayout->addLayout(teamInnerLayout);
    };

    //Create Team Boxes
    QFrame* gameTeamAFrame;
    QFrame* gameTeamBFrame;
    QLabel* teamATotalLabel;
    QLabel* teamBTotalLabel;
    QListWidget* gameTeamANameList;
    QListWidget* gameTeamAScoreList;
    QListWidget* gameTeamBNameList;
    QListWidget* gameTeamBScoreList;

    createTeamBox("Team A", "#922f34", gameTeamAFrame, teamATotalLabel, gameTeamANameList, gameTeamAScoreList);
    createTeamBox("Team B", "#4fb269", gameTeamBFrame, teamBTotalLabel, gameTeamBNameList, gameTeamBScoreList);

    gameBoxesLayout.addWidget(gameTeamAFrame, 1);
    gameBoxesLayout.addWidget(gameTeamBFrame, 1);
    gameLayout.addLayout(&gameBoxesLayout);


//---------------- Timer Logic ----------------
    enum class GameState { Idle, Cooldown, Running, Paused, Ended };
    GameState gameState = GameState::Idle;

    //Score Flash
    QTimer flashTimer;
    flashTimer.setInterval(500);
    bool flashVisible = true;
    QString currentLeader = "";

    //Main Timer
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
    std::function<void()> updateTeamDisplays;

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

        updateTeamDisplays();

        updateTimerLabel();
        gameStatusBox.append("Game ended.");
		
		const char* NA = qPrintable(currentIp);
		std::cout << "Closing Game..." << std::endl;
		Broadcast("221", NA);
		Broadcast("221", NA);
		Broadcast("221", NA);
    };

    //Cooldown Timer
    startCooldownThenRun = [&]()
    {
        cooldownRemaining = 30;
        gameState = GameState::Cooldown;
        updateTimerLabel();
        tick.start();
    };

    //Game Timer
    QObject::connect(&tick, &QTimer::timeout, [&]()
    {
        if (gameState == GameState::Cooldown)
        {
			syncScores(teamAPlayers, teamBPlayers, scorecard);
			updateTeamDisplays();
			eventHandler(scorecard, &gameStatusBox, teamAPlayers, teamBPlayers);
            cooldownRemaining--;
            if (cooldownRemaining <= 0)
            {
                gameRemaining = (savedGameRemaining > 0 ? savedGameRemaining : 360);
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
			syncScores(teamAPlayers, teamBPlayers, scorecard);
			updateTeamDisplays();
			eventHandler(scorecard, &gameStatusBox, teamAPlayers, teamBPlayers);
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

    //Team Score Update
    updateTeamDisplays = [&]()
    {
        auto sortByScore = [](const Player& a, const Player& b)
        {
            return a.score > b.score;
        };

        std::sort(teamAPlayers.begin(), teamAPlayers.end(), sortByScore);
        std::sort(teamBPlayers.begin(), teamBPlayers.end(), sortByScore);

        gameTeamANameList->clear();
        gameTeamAScoreList->clear();
        gameTeamBNameList->clear();
        gameTeamBScoreList->clear();

        int teamASum = 0, teamBSum = 0;
        for (const auto& p : teamAPlayers)
        {
            QListWidgetItem* nameItem = new QListWidgetItem(QString("%1 (%2)").arg(p.codename).arg(p.status));
            nameItem->setData(Qt::UserRole, p.codename);          // store codename for reliable lookup
            gameTeamANameList->addItem(nameItem);

            QListWidgetItem* scoreItem = new QListWidgetItem(QString::number(p.score));
            scoreItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            gameTeamAScoreList->addItem(scoreItem);

            teamASum += p.score;
        }
        for (const auto& p : teamBPlayers)
        {
            QListWidgetItem* nameItem = new QListWidgetItem(QString("%1 (%2)").arg(p.codename).arg(p.status));
            nameItem->setData(Qt::UserRole, p.codename);
            gameTeamBNameList->addItem(nameItem);

            QListWidgetItem* scoreItem = new QListWidgetItem(QString::number(p.score));
            scoreItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            gameTeamBScoreList->addItem(scoreItem);

            teamBSum += p.score;
        }

        teamATotalLabel->setText(QString("Score: %1").arg(teamASum));
        teamBTotalLabel->setText(QString("Score: %1").arg(teamBSum));

        teamATotalLabel->setText(QString("Score: %1").arg(teamASum));
        teamBTotalLabel->setText(QString("Score: %1").arg(teamBSum));


        //Flashing Logic
        QString newLeader;
        if (teamASum > teamBSum) newLeader = "A";
        else if (teamBSum > teamASum) newLeader = "B";
        else newLeader = "";

        if (newLeader != currentLeader)
        {
            currentLeader = newLeader;
            flashVisible = true;
            flashTimer.stop();

            teamATotalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");
            teamBTotalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");

            if (currentLeader != "")
                flashTimer.start();
        }
    };


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
        clearIDS(scorecard);
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
		addID(scorecard, equipmentid);
		
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
		Broadcast(BID, NA);
		playerIdInput.clear();
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
		
		int equipmentID = -1;

		auto findPlayerEqID = [&](const QList<Player>& team) -> bool {
			for (const auto& p : team) {
				if (p.id == enteredId) {
					equipmentID = p.EqID;
					return true;
				}
			}
			return false;
		};
		findPlayerEqID(teamAPlayers) || findPlayerEqID(teamBPlayers);


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

        removeID(scorecard, equipmentID);
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
	
	//Disable Receiver Timeout
	QObject::connect(&allowTimeoutBtn, &QPushButton::clicked, [&] ()
	{
		if(timeoutcheck == "true"){
			timeoutcheck = "false";
			statusBox.append("Receiver timeout disabled - [Manually close the window before launching new game]");
		}
		else if(timeoutcheck == "false"){
			timeoutcheck = "true";
			statusBox.append("Receiver timeout enabled");
		}
	});

    //Ready Game
    QObject::connect(&readyGameBtn, &QPushButton::clicked, [&]()
    {
        updateTeamDisplays();

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

            updateTeamDisplays();

            teamAList.clear();
            for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
            teamBList.clear();
            for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

            startCooldownThenRun();
            gameStatusBox.append("Game starting.");
            gameStartBtn.setText("Restart Game");
			
			//play random track
            system("./randomtrack &");
        }
        else
        {
            for (auto& p : teamAPlayers) p.status = "Active";
            for (auto& p : teamBPlayers) p.status = "Active";

            updateTeamDisplays();

            teamAList.clear();
            for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
            teamBList.clear();
            for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

            startCooldownThenRun();
            gameStatusBox.append("Game restarted.");
            gameStartBtn.setText("Restart Game");
			
			//play random track
            system("./randomtrack &");
        }
		const char* NA = qPrintable(currentIp);
		std::string command1 = std::string("xterm -hold -e './Game.exe 1 ") + NA + " " + timeoutcheck + "' &";
		int sys_check_start = system(command1.c_str());
		std::string command2 = std::string("xterm -hold -e './Game.exe 2 ") + NA + " " + timeoutcheck +"' &";
		int sys_check_start2 = system(command2.c_str());
		clearScores(scorecard);
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

        updateTeamDisplays();

        updateTimerLabel();
        gameScreen.hide();
        mainScreen.show();

        gameStartBtn.setText("Start");
        gamePauseBtn.setText("Pause");
        gameStatusBox.append("Game ended and returning to main screen.");
        
		const char* NA = qPrintable(currentIp);
		std::cout << "Closing Game..." << std::endl;
		Broadcast("221", NA);
		Broadcast("221", NA);
		Broadcast("221", NA);
    });


//Player Event Testing (Double-Click Players to Change)
    //Team A Handler
    QObject::connect(gameTeamANameList, &QListWidget::itemDoubleClicked, [&](QListWidgetItem* item)
    {
        QString codename = item->data(Qt::UserRole).toString();

        for (auto& p : teamAPlayers)
        {
            if (p.codename == codename)
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

        updateTeamDisplays();
    });

    //Team B Handler
    QObject::connect(gameTeamBNameList, &QListWidget::itemDoubleClicked, [&](QListWidgetItem* item)
    {
        QString codename = item->data(Qt::UserRole).toString();

        for (auto& p : teamBPlayers)
        {
            if (p.codename == codename)
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

        updateTeamDisplays();
    });

    //Team Score Flashing
    QObject::connect(&flashTimer, &QTimer::timeout, [&]()
    {
        flashVisible = !flashVisible;

        auto flashStyle = [&](QLabel* label)
        {
            if (flashVisible)
                label->setStyleSheet("background: transparent; border: none; color: #ffea00; font-weight: bold;"); // yellow highlight
            else
                label->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");
        };

        if (currentLeader == "A")
        {
            flashStyle(teamATotalLabel);
            teamBTotalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");
        }
        else if (currentLeader == "B")
        {
            flashStyle(teamBTotalLabel);
            teamATotalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");
        }
        else
        {
            teamATotalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");
            teamBTotalLabel->setStyleSheet("background: transparent; border: none; color: #f0f0f0; font-weight: bold;");
        }
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
