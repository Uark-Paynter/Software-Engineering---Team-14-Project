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

//Player Framework
struct Player
{
    int id;
    QString codename;
    QString status;
    int score = 0;
};

//Text Helper
QString formatPlayer(const Player& p)
{
    return QString("ID:%1  %2  - %3").arg(p.id).arg(p.codename).arg(p.status);
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

    //Database of available players
    QList<Player> availablePlayers =
    {
        {101, "Alice", "Ready"},
        {202, "Bob", "Ready"},
        {303, "Charlie", "Ready"},
        {404, "Xander", "Ready"},
        {505, "Yara", "Ready"},
        {606, "Zane", "Ready"}
    };



    //---------------- Main Screen ----------------
    QWidget mainScreen;
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


    //---------------- Network Buttons (Fake IP) ----------------
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
    QLabel currentIpLabel("Current IP: 124.13.219.81");
    currentIpLabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    currentIpLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    currentIpLabel.setMinimumWidth(200);
    networkLayout.addWidget(&currentIpLabel);
    QString currentIp = "124.13.219.81"; //Default IP setter

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
        statusBox.append("Players cleared from both teams.");
    });

    //Add Player
    QObject::connect(&addPlayerBtn, &QPushButton::clicked, [&]()
    {
        bool ok;
        int enteredId = playerIdInput.text().toInt(&ok);
        if (!ok)
        {
            statusBox.append("Invalid ID entered.");
            return;
        }

        auto it = std::find_if(availablePlayers.begin(), availablePlayers.end(), [&](const Player& p){ return p.id == enteredId; });

        if (it == availablePlayers.end())
        {
            statusBox.append("Player with ID " + QString::number(enteredId) + " not found.");
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

        if (enteredId % 2 == 1)
        {
            //Team A
            Player newp = *it;
            newp.status = "Ready";
            teamAPlayers.append(newp);
            teamAList.addItem(formatPlayer(newp));
            statusBox.append(it->codename + " added to Team A.");
        }
        else
        {
            //Team B
            Player newp = *it;
            newp.status = "Ready";
            teamBPlayers.append(newp);
            teamBList.addItem(formatPlayer(newp));
            statusBox.append(it->codename + " added to Team B.");
        }
    });

    //Remove Player
    QObject::connect(&removePlayerBtn, &QPushButton::clicked, [&]()
    {
        bool ok;
        int enteredId = playerIdInput.text().toInt(&ok);
        if (!ok)
        {
            statusBox.append("Invalid ID entered.");
            return;
        }

        auto removeFromTeam = [&](QList<Player>& team, QListWidget& list, const QString& teamName)
        {
            for (int i = 0; i < team.size(); ++i)
            {
                if (team[i].id == enteredId)
                {
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
            statusBox.append("Player with ID " + QString::number(enteredId) + " not found in game.");
        }
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
                    p.score += 10;
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
                    p.score += 10;
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
