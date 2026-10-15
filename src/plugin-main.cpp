// SPDX-License-Identifier: GPL-2.0-or-later
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/platform.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QtEndian>
#include <QDateTime>
#include <QGroupBox>
#include <QTabWidget>
#include <QToolButton>
#include <QStackedWidget>
#include <QListWidget>
#include <QPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollArea>
#include <array>
#include <algorithm>
#include "reminder.hpp"
#include "audio-watch.hpp"
#include "sound-preset.hpp"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif
OBS_DECLARE_MODULE()

MODULE_EXPORT const char *obs_module_description(void) { return "Clip and microphone notifications"; }
namespace {
constexpr std::array<const char *, 10> keys{"replay", "mute", "unmute", "reminder", "replay_start", "replay_stop", "mic_silent", "desktop_silent", "black_screen", "frozen_screen"};
constexpr std::array<const char *, 10> titles{"Replay saved", "Microphone muted", "Microphone unmuted", "Muted reminder", "Replay buffer started", "Replay buffer stopped", "Microphone: no signal", "Desktop: no signal", "Possible black screen", "Possible frozen capture"};
struct Sound { QString path; int volume = 100; bool enabled = true; };
class Companion : public QObject {
    QTimer timer;
    QSettings settings;
    QByteArray playing;
    QString mic;
    bool enabled = true, reminders = true, known = false, lastMuted = false;
    int first = 60, repeat = 60;
    Reminder reminder;
    std::array<Sound, 10> sounds;
    AudioWatch micWatch, desktopWatch;
    QString soundStyle="minimal", voiceLanguage="auto";
    QString language="en";
    QJsonObject translation;
    QString trText(const char *text) const { return translation.value(QString::fromUtf8(text)).toString(QString::fromUtf8(text)); }
    void loadLanguage() {
        QString code=language=="auto" ? QString::fromUtf8(obs_get_locale()) : language;
        code=code.left(2).toLower();
        QFile file(":/companion/i18n/"+code+".json");
        translation={}; if(file.open(QIODevice::ReadOnly)) translation=QJsonDocument::fromJson(file.readAll()).object();
    }
    void readWatch(AudioWatch &watch, const QString &prefix) {
        watch.name=settings.value(prefix+"/source").toString();
        watch.enabled=settings.value(prefix+"/enabled",false).toBool();
        watch.threshold=std::clamp(settings.value(prefix+"/threshold",-50).toInt(),-90,-5);
        watch.delay=std::clamp(settings.value(prefix+"/delay",prefix=="desktopSignal" ? 120 : 30).toInt(),5,3600);
        watch.cooldown=std::clamp(settings.value(prefix+"/cooldown",60).toInt(),5,3600);
    }
    void writeWatch(const AudioWatch &watch,const QString &prefix) {
        settings.setValue(prefix+"/source",watch.name); settings.setValue(prefix+"/enabled",watch.enabled);
        settings.setValue(prefix+"/threshold",watch.threshold); settings.setValue(prefix+"/delay",watch.delay);
        settings.setValue(prefix+"/cooldown",watch.cooldown);
    }
    QLabel *status = nullptr;
    bool panelEnabled=false;
    QPointer<QWidget> panel;
    QPointer<QListWidget> notifications;
    QPointer<QLabel> panelStatus;
    QPointer<QPushButton> panelPause;
    QLabel *monitorStatus=nullptr; QPointer<QPushButton> pauseButton,resumeButton;
    std::int64_t pausedUntil=0;
    void notify(size_t index) {
        if(!enabled || !notifications) return;
        notifications->insertItem(0,QDateTime::currentDateTime().toString("HH:mm:ss")+"  "+trText(titles[index]));
        while(notifications->count()>30) delete notifications->takeItem(notifications->count()-1);
    }
    void updatePanel() {
        if(panelEnabled && !panel) {
            auto *widget=new QWidget; auto *layout=new QVBoxLayout(widget);
            panelStatus=new QLabel; panelStatus->setWordWrap(true); layout->addWidget(panelStatus);
            notifications=new QListWidget; layout->addWidget(notifications);
            auto *pause=new QPushButton(trText("Pause all monitoring for 5 minutes")); panelPause=pause; layout->addWidget(pause);
            connect(pause,&QPushButton::clicked,this,[this] { setPaused(now()<pausedUntil ? 0 : now()+300); });
            if(obs_frontend_add_dock_by_id("obs-companion-notifications","OBS Companion",widget)) panel=widget;
            else { delete widget; notifications=nullptr; panelStatus=nullptr; }
        } else if(!panelEnabled && panel) {
            obs_frontend_remove_dock("obs-companion-notifications"); panel=nullptr; notifications=nullptr; panelStatus=nullptr;
        }
        if(panelPause) panelPause->setText(now()<pausedUntil ? trText("Resume all monitoring") : trText("Pause all monitoring for 5 minutes"));
    }
    bool dialogOpen = false;
    std::int64_t now() const { return static_cast<std::int64_t>(os_gettime_ns() / 1000000000ULL); }
    void setPaused(std::int64_t until) { pausedUntil=until; resetCaptureTimer(); tick(); }
public:
    Companion(const QString &path) : settings(path, QSettings::IniFormat) {
        language=settings.value("language","en").toString(); loadLanguage();
        readWatch(micWatch,"micSignal"); readWatch(desktopWatch,"desktopSignal");
        soundStyle=settings.value("soundStyle","minimal").toString();
        voiceLanguage=settings.value("voiceLanguage","auto").toString();
        settings.remove("video");
        enabled = settings.value("enabled", true).toBool();
        reminders = settings.value("reminders", true).toBool();
        mic = settings.value("microphone").toString();
        if(mic.isEmpty() && !micWatch.name.isEmpty()) mic=micWatch.name;
        micWatch.name=mic;
        panelEnabled=settings.value("notificationPanel",false).toBool(); updatePanel();
        first = std::clamp(settings.value("first", 60).toInt(), 5, 3600);
        repeat = std::clamp(settings.value("repeat", 60).toInt(), 5, 3600);
        for (size_t i = 0; i < sounds.size(); ++i) {
            QString key = QString::fromUtf8(keys[i]);
            sounds[i].path = settings.value(key + "/path").toString();
            sounds[i].volume = std::clamp(settings.value(key + "/volume", 100).toInt(), 0, 100);
            sounds[i].enabled = settings.value(key + "/enabled", true).toBool();
        }
        connect(&timer, &QTimer::timeout, this, [this] { tick(); });
        timer.start(250);
    }
    ~Companion() override {
        panelEnabled=false; updatePanel();
        timer.stop();
        micWatch.clear(); desktopWatch.clear();
#ifdef _WIN32
        PlaySoundW(nullptr, nullptr, 0);
#endif
    }
    void play(size_t index, const Sound *test = nullptr, bool audible=true,
              const QString &previewStyle=QString(), const QString &previewVoice=QString(), const QString &previewLanguage=QString()) {
        const Sound &sound = test ? *test : sounds[index];
        if(!test) {
            if(!enabled) return;
            notify(index);
            if(!sound.enabled || !audible) return;
        }
        QString path = sound.path;
        if(path.isEmpty()) {
            const auto style=(previewStyle.isNull() ? soundStyle : previewStyle).toStdString();
            const auto voice=(previewVoice.isNull() ? voiceLanguage : previewVoice).toStdString();
            const auto ui=(previewLanguage.isNull() ? language : previewLanguage).toStdString();
            path=QString::fromStdString(presetResource(style,voice,ui,obs_get_locale(),keys[index]));
            if(!QFile::exists(path)) path=QString(":/companion/sounds/")+keys[index]+".wav";
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly) || file.size() > 4 * 1024 * 1024) {
            soundError(test,"Cannot read WAV. Check its path and size (maximum 4 MiB)."); return;
        }
        auto data = file.readAll();
        // Accept bounded 16-bit PCM WAV files; reject compressed or malformed inputs.
        if (data.size() < 44 || data.left(4) != "RIFF" || data.mid(8, 4) != "WAVE") { soundError(test,"Use a mono/stereo 16-bit PCM WAV."); return; }
        bool format = false; qsizetype audioStart = 0, audioSize = 0;
        for (qsizetype pos = 12; pos + 8 <= data.size();) {
            auto len = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(data.constData() + pos + 4));
            const qsizetype start = pos + 8;
            if (len > static_cast<quint64>(data.size() - start)) { soundError(test,"Use a mono/stereo 16-bit PCM WAV."); return; }
            if (data.mid(pos, 4) == "fmt " && len >= 16) {
                auto u16 = [&](int offset) { return qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(data.constData() + start + offset)); };
                format = u16(0) == 1 && (u16(2) == 1 || u16(2) == 2) && u16(14) == 16;
            }
            if (data.mid(pos, 4) == "data") { audioStart = start; audioSize = len; }
            pos = start + len + (len & 1U);
        }
        if (!format || !audioStart || !audioSize || audioSize % 2) {
            soundError(test,"Use a mono/stereo 16-bit PCM WAV."); return;
        }
        for (qsizetype i = audioStart; i < audioStart + audioSize; i += 2) {
            const auto value = qFromLittleEndian<qint16>(reinterpret_cast<const uchar *>(data.constData() + i));
            qToLittleEndian<qint16>(static_cast<qint16>(value * sound.volume / 100), reinterpret_cast<uchar *>(data.data() + i));
        }
#ifdef _WIN32
        PlaySoundW(nullptr, nullptr, 0); // Stop before replacing async buffer storage.
        playing = std::move(data);
        if (!PlaySoundW(reinterpret_cast<LPCWSTR>(playing.constData()), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT))
            soundError(test,"Windows could not play this sound.");
#else
        blog(LOG_INFO, "[OBS Companion] Audio playback is implemented for Windows only");
#endif
    }
    void soundError(const Sound *test,const char *message) {
        blog(LOG_WARNING,"[OBS Companion] %s",message);
        if(test) QMessageBox::warning(static_cast<QWidget *>(obs_frontend_get_main_window()),"OBS Companion",trText(message));
    }
    void tick() {
        const bool capture=obs_frontend_replay_buffer_active() ||
            (obs_frontend_recording_active() && !obs_frontend_recording_paused());
        const bool active=enabled && capture && now()>=pausedUntil;
        std::array<bool,10> pending{};
        obs_source_t *source=mic.isEmpty() ? nullptr : obs_get_source_by_name(mic.toUtf8().constData());
        QString micState=trText(mic.isEmpty() ? "None" : "Selected microphone is unavailable.");
        if(source) {
            bool muted=obs_source_muted(source); obs_source_release(source);
            if(known && muted!=lastMuted) { pending[muted ? 1 : 2]=true; resetAudioTimers(); }
            known=true; lastMuted=muted;
            pending[3]=reminder.step(now(),active && reminders,muted,first,repeat);
            micState=trText(muted ? "Microphone is muted" : "Microphone is unmuted");
        } else { known=false; reminder.reset(); }
        if(status) status->setText(micState);
        pending[6]=micWatch.step(now(),active);
        pending[7]=desktopWatch.step(now(),active);
        // Keep all visual events, but play only one sound per timer cycle.
        std::array<bool,10> soundEnabled{};
        for(size_t i=0;i<sounds.size();++i) soundEnabled[i]=sounds[i].enabled;
        auto audible=chooseAlert(pending,soundEnabled);
        for(size_t i=0;i<pending.size();++i) if(pending[i]) play(i,nullptr,audible && *audible==i);
        QString state;
        const auto remaining=std::max<std::int64_t>(0,pausedUntil-now());
        const QString clock=QString("%1:%2").arg(remaining/60,2,10,QChar('0')).arg(remaining%60,2,10,QChar('0'));
        if(!enabled) state=trText("Monitoring off: notifications are disabled");
        else if(remaining>0) state=trText("Monitoring paused — resumes in %1").arg(clock);
        else if(!capture) state=trText("Monitoring on — waiting for Replay Buffer or recording");
        else state=trText("Monitoring active — Replay Buffer or recording");
        if(monitorStatus) monitorStatus->setText(state);
        if(pauseButton) pauseButton->setVisible(remaining==0);
        if(resumeButton) resumeButton->setVisible(remaining>0);
        if(panelPause) panelPause->setText(remaining>0 ? trText("Resume all monitoring") : trText("Pause all monitoring for 5 minutes"));
        if(panelStatus) panelStatus->setText(state+"\n"+micState);
    }
    void resetAudioTimers() { reminder.reset(); micWatch.alert.reset(); desktopWatch.alert.reset(); }
    void resetCaptureTimer() { resetAudioTimers(); }
    void shutdown() { timer.stop(); micWatch.clear(); desktopWatch.clear(); }
    void dialog() {
        if(dialogOpen) return;
        dialogOpen=true;
        QDialog window(static_cast<QWidget *>(obs_frontend_get_main_window()));
        window.setWindowTitle("OBS Companion 1.1.0"); window.resize(820,640);
        auto *layout=new QVBoxLayout(&window); auto *tabs=new QTabWidget; layout->addWidget(tabs);
        auto tab=[&](const char *name) {
            auto *scroll=new QScrollArea; scroll->setWidgetResizable(true); auto *body=new QWidget;
            auto *box=new QVBoxLayout(body); box->setAlignment(Qt::AlignTop); scroll->setWidget(body);
            tabs->addTab(scroll,trText(name)); return box;
        };
        auto group=[&](QVBoxLayout *parent,const char *name) {
            auto *box=new QGroupBox(trText(name)); auto font=box->font(); font.setBold(true); box->setFont(font);
            auto *content=new QWidget; content->setFont(window.font()); auto *outer=new QVBoxLayout(box); outer->addWidget(content);
            auto *form=new QFormLayout(content);
            box->setStyleSheet("QGroupBox::title { font-weight: bold; }");
            parent->addWidget(box); return form;
        };
        auto info=[&](QFormLayout *form,const char *message) {
            auto *label=new QLabel(trText(message)); label->setWordWrap(true); label->setFont(window.font()); form->addRow(label);
        };
        auto toggle=[&](QFormLayout *form,const char *label,bool value) {
            auto *v=new QCheckBox(trText(label)); v->setFont(window.font()); v->setChecked(value); form->addRow(v); return v;
        };
        auto advanced=[&](QFormLayout *parent,bool soundOptions=false) {
            auto *button=new QToolButton; button->setText(trText(soundOptions ? "Customize sounds" : "Show advanced settings")); button->setCheckable(true);
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); button->setArrowType(Qt::RightArrow); parent->addRow(button);
            auto *body=new QWidget; body->setFont(window.font()); auto *form=new QFormLayout(body); body->setVisible(false); parent->addRow(body);
            connect(button,&QToolButton::toggled,&window,[=](bool show) {
                body->setVisible(show); button->setArrowType(show ? Qt::DownArrow : Qt::RightArrow);
                button->setText(trText(soundOptions ? (show ? "Hide sound settings" : "Customize sounds") : (show ? "Hide advanced settings" : "Show advanced settings")));
            }); return form;
        };
        auto spin=[&](QFormLayout *form,const char *label,int value,int low,int high,const QString &suffix) {
            auto *v=new QSpinBox; v->setFont(window.font()); v->setRange(low,high); v->setValue(value); v->setSuffix(suffix);
            form->addRow(trText(label),v); return v;
        };
        auto sourceBox=[&](QFormLayout *form,const char *label,const QString &selected) {
            auto *v=new QComboBox; v->setFont(window.font()); v->addItem(trText("None"),QString());
            obs_enum_sources([](void *param,obs_source_t *source) {
                if(obs_source_get_output_flags(source)&OBS_SOURCE_AUDIO) {
                    auto name=QString::fromUtf8(obs_source_get_name(source)); static_cast<QComboBox *>(param)->addItem(name,name);
                } return true;
            },v);
            if(!selected.isEmpty() && v->findData(selected)<0) v->addItem(selected+" ("+trText("Unavailable")+")",selected);
            v->setCurrentIndex(std::max(0,v->findData(selected))); form->addRow(trText(label),v); return v;
        };
        auto draft=sounds;
        bool resetAllDraft=false;
        QComboBox *styles=nullptr,*voiceLanguages=nullptr,*languages=nullptr;
        std::array<QCheckBox *,10> soundSwitches{};
        std::array<QLineEdit *,10> soundPaths{};
        std::array<QSpinBox *,10> soundVolumes{};
        auto soundEditor=[&](QFormLayout *parent,std::initializer_list<size_t> indices) {
            auto *form=advanced(parent,true); info(form,"Custom WAV overrides the selected preset. Leave the path blank to use the preset.");
            auto *select=new QComboBox; auto *stack=new QStackedWidget;
            for(size_t i:indices) {
                select->addItem(trText(titles[i])); auto *page=new QWidget; auto *rows=new QFormLayout(page);
                auto *on=toggle(rows,"Enable this sound",draft[i].enabled);
                auto *path=new QLineEdit(draft[i].path); path->setPlaceholderText(trText("Preset sound (leave blank)"));
                auto *buttons=new QHBoxLayout; buttons->addWidget(path);
                auto *browse=new QPushButton(trText("Browse")); buttons->addWidget(browse);
                auto *reset=new QPushButton(trText("Use preset sound")); buttons->addWidget(reset); rows->addRow(trText("Sound file"),buttons);
                auto *volume=spin(rows,"Volume",draft[i].volume,0,100,"%");
                auto *test=new QPushButton(trText("Test")); rows->addRow(test);
                soundSwitches[i]=on; soundPaths[i]=path; soundVolumes[i]=volume;
                connect(on,&QCheckBox::toggled,&window,[&,i](bool value) { draft[i].enabled=value; });
                connect(path,&QLineEdit::textChanged,&window,[&,i](const QString &value) { draft[i].path=value; });
                connect(volume,qOverload<int>(&QSpinBox::valueChanged),&window,[&,i](int value) { draft[i].volume=value; });
                connect(reset,&QPushButton::clicked,&window,[path] { path->clear(); });
                connect(browse,&QPushButton::clicked,&window,[&,path] {
                    auto selected=QFileDialog::getOpenFileName(&window,trText("16-bit PCM WAV"),{},"WAV audio (*.wav)");
                    if(!selected.isEmpty()) path->setText(selected);
                });
                connect(test,&QPushButton::clicked,&window,[&,i] { play(i,&draft[i],true,styles->currentData().toString(),voiceLanguages->currentData().toString(),languages->currentData().toString()); }); stack->addWidget(page);
            }
            form->addRow(trText("Choose notification sound"),select); form->addRow(stack);
            connect(select,qOverload<int>(&QComboBox::currentIndexChanged),stack,&QStackedWidget::setCurrentIndex);
        };
        auto *general=tab("General"); auto *preferences=group(general,"Preferences");
        auto *master=toggle(preferences,"Enable notifications",enabled);
        languages=new QComboBox; languages->addItem(trText("Follow OBS"),"auto"); languages->addItem("English","en");
        languages->addItem(QString::fromUtf8("Español"),"es"); languages->addItem(QString::fromUtf8("中文（简体）"),"zh");
        languages->addItem(QString::fromUtf8("Français"),"fr"); languages->setCurrentIndex(std::max(0,languages->findData(language)));
        preferences->addRow(trText("Language (apply and reopen)"),languages);
        auto *soundPreferences=group(general,"Sound presets");
        styles=new QComboBox; styles->addItem(trText("Minimalist tones"),"minimal"); styles->addItem(trText("Voices"),"voice");
        styles->setCurrentIndex(std::max(0,styles->findData(soundStyle))); soundPreferences->addRow(trText("Sound style"),styles);
        voiceLanguages=new QComboBox; voiceLanguages->addItem(trText("Follow interface language"),"auto");
        voiceLanguages->addItem("English","en"); voiceLanguages->addItem(QString::fromUtf8("Español"),"es");
        voiceLanguages->addItem(QString::fromUtf8("Français"),"fr"); voiceLanguages->addItem(QString::fromUtf8("中文"),"zh");
        voiceLanguages->setCurrentIndex(std::max(0,voiceLanguages->findData(voiceLanguage)));
        soundPreferences->addRow(trText("Voice language"),voiceLanguages); voiceLanguages->setEnabled(styles->currentData()=="voice");
        connect(styles,qOverload<int>(&QComboBox::currentIndexChanged),&window,[=] { voiceLanguages->setEnabled(styles->currentData()=="voice"); });
        info(soundPreferences,"Custom WAV files take priority. Changing presets preserves custom files, volumes and disabled sounds.");
        auto *restoreSounds=new QPushButton(trText("Restore sound defaults")); soundPreferences->addRow(restoreSounds);
        auto *resetAll=new QPushButton(trText("Reset all settings")); soundPreferences->addRow(resetAll);
        auto *showPanel=toggle(preferences,"Show notification panel inside OBS",panelEnabled);
        info(preferences,"The panel stays inside OBS. No desktop popups or game overlay.");
        info(preferences,"You can disable individual sounds or pause monitoring if reminders become distracting.");
        auto *monitorGroup=group(general,"Monitoring status");
        monitorStatus=new QLabel; monitorStatus->setWordWrap(true); monitorStatus->setStyleSheet("font-weight: bold; padding: 8px;"); monitorGroup->addRow(trText("Current state"),monitorStatus);
        info(monitorGroup,"Status updates while OBS is open. Pausing stops duration-based reminders for five minutes.");
        pauseButton=new QPushButton(trText("Pause all monitoring for 5 minutes")); preferences->addRow(pauseButton);
        connect(pauseButton,&QPushButton::clicked,&window,[this] { setPaused(now()+300); });
        resumeButton=new QPushButton(trText("Resume all monitoring")); preferences->addRow(resumeButton); resumeButton->hide();
        connect(resumeButton,&QPushButton::clicked,&window,[this] { setPaused(0); });
        auto *allVolume=new QPushButton(trText("Set all sound volumes to 100%")); preferences->addRow(allVolume);
        // Created after editors; invoke their controls through a shared signal-free list below.
        info(preferences,"New sounds default to 100%. Existing custom volume settings are preserved.");
        info(preferences,"Sounds use the Windows default output and may enter Desktop Audio.\nCustom audio: mono/stereo 16-bit PCM WAV, at most 4 MiB.");
        auto *replayTab=tab("Replay"); auto *replayForm=group(replayTab,"Replay notifications");
        info(replayForm,"Saved, started and stopped sounds use OBS events. Your existing hotkeys stay unchanged.");
        soundEditor(replayForm,{0,4,5});
        auto *microphoneTab=tab("Microphone"); auto *microphoneForm=group(microphoneTab,"Microphone monitoring");
        auto *sources=sourceBox(microphoneForm,"Microphone source",mic);
        auto *live=new QLabel; live->setWordWrap(true); status=live;
        microphoneForm->addRow(trText("Current microphone state"),live);
        info(microphoneForm,"None disables microphone tracking. Mute detection and no-signal checks share this source.");
        auto *remind=toggle(microphoneForm,"Enable muted microphone reminders",reminders);
        info(microphoneForm,"Only enables reminders; does not mute or disable your microphone.");
        auto *micSignal=toggle(microphoneForm,"Enable no-signal reminders",micWatch.enabled);
        info(microphoneForm,"Only need mute reminders? Leave no-signal monitoring off. Silence does not prove device failure.");
        info(microphoneForm,"Muting suspends no-signal checks. Disabling mute reminders never enables no-signal monitoring.");
        auto *micAdvanced=advanced(microphoneForm);
        auto *initial=spin(micAdvanced,"First reminder after",first,5,3600," s");
        auto *interval=spin(micAdvanced,"Repeat reminder every",repeat,5,3600," s");
        auto *micThreshold=spin(micAdvanced,"Signal threshold",micWatch.threshold,-90,-5," dBFS");
        auto *micDelay=spin(micAdvanced,"No signal for",micWatch.delay,5,3600," s");
        auto *micRepeat=spin(micAdvanced,"Repeat warning every",micWatch.cooldown,5,3600," s");
        soundEditor(microphoneForm,{1,2,3,6});
        auto *desktopTab=tab("Desktop"); auto *desktopForm=group(desktopTab,"Desktop audio monitoring");
        auto *desktopSource=sourceBox(desktopForm,"Audio source",desktopWatch.name);
        auto *desktopSignal=toggle(desktopForm,"Enable no-signal reminders",desktopWatch.enabled);
        info(desktopForm,"Quiet games and pauses can trigger this reminder. Disable it when silence is intentional.");
        auto *desktopAdvanced=advanced(desktopForm);
        auto *desktopThreshold=spin(desktopAdvanced,"Signal threshold",desktopWatch.threshold,-90,-5," dBFS");
        auto *desktopDelay=spin(desktopAdvanced,"No signal for",desktopWatch.delay,5,3600," s");
        auto *desktopRepeat=spin(desktopAdvanced,"Repeat warning every",desktopWatch.cooldown,5,3600," s");
        soundEditor(desktopForm,{7});
        connect(allVolume,&QPushButton::clicked,&window,[&,body=&window] {
            for(auto &sound:draft) sound.volume=100;
            for(auto *v:body->findChildren<QSpinBox *>()) if(v->suffix()=="%") v->setValue(100);
        });
        auto restoreSoundControls=[&] {
            styles->setCurrentIndex(0); voiceLanguages->setCurrentIndex(0);
            for(size_t i=0;i<draft.size();++i) {
                draft[i]=Sound{};
                if(soundSwitches[i]) soundSwitches[i]->setChecked(true);
                if(soundPaths[i]) soundPaths[i]->clear();
                if(soundVolumes[i]) soundVolumes[i]->setValue(100);
            }
        };
        connect(restoreSounds,&QPushButton::clicked,&window,[&] {
            if(QMessageBox::question(&window,trText("Restore sound defaults"),trText("Reset sounds to minimalist tones at 100%, enable sounds and clear custom paths? Files will not be deleted. Apply to save."),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes) restoreSoundControls();
        });
        connect(resetAll,&QPushButton::clicked,&window,[&] {
            if(QMessageBox::question(&window,trText("Reset all settings"),trText("Restore all plugin settings? Custom files will not be deleted. Apply to save, or Cancel to discard."),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes) return;
            resetAllDraft=true; restoreSoundControls(); master->setChecked(true); remind->setChecked(true);
            languages->setCurrentIndex(languages->findData("en")); showPanel->setChecked(false);
            sources->setCurrentIndex(0); desktopSource->setCurrentIndex(0);
            initial->setValue(60); interval->setValue(60); micSignal->setChecked(false); desktopSignal->setChecked(false);
            micThreshold->setValue(-50); micDelay->setValue(30); micRepeat->setValue(60);
            desktopThreshold->setValue(-50); desktopDelay->setValue(120); desktopRepeat->setValue(60);
        });
        tick();
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Apply|QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Save)->setText(trText("Save and close"));
        buttons->button(QDialogButtonBox::Apply)->setText(trText("Apply")); buttons->button(QDialogButtonBox::Cancel)->setText(trText("Cancel"));
        layout->addWidget(buttons); connect(buttons,&QDialogButtonBox::accepted,&window,&QDialog::accept);
        connect(buttons,&QDialogButtonBox::rejected,&window,&QDialog::reject);
        auto applySettings=[&] {
            if(resetAllDraft) { settings.clear(); pausedUntil=0; if(notifications) notifications->clear(); resetAllDraft=false; }
            soundStyle=styles->currentData().toString(); voiceLanguage=voiceLanguages->currentData().toString();
            settings.setValue("soundStyle",soundStyle); settings.setValue("voiceLanguage",voiceLanguage);
            settings.remove("video");
            enabled=master->isChecked(); reminders=remind->isChecked(); mic=sources->currentData().toString();
            first=initial->value(); repeat=interval->value(); known=false;
            micWatch.clear(); micWatch.name=mic; micWatch.enabled=micSignal->isChecked();
            micWatch.threshold=micThreshold->value(); micWatch.delay=micDelay->value(); micWatch.cooldown=micRepeat->value();
            desktopWatch.clear(); desktopWatch.name=desktopSource->currentData().toString(); desktopWatch.enabled=desktopSignal->isChecked();
            desktopWatch.threshold=desktopThreshold->value(); desktopWatch.delay=desktopDelay->value(); desktopWatch.cooldown=desktopRepeat->value();
            writeWatch(micWatch,"micSignal"); writeWatch(desktopWatch,"desktopSignal");
            settings.setValue("enabled",enabled); settings.setValue("reminders",reminders); settings.setValue("microphone",mic);
            settings.setValue("first",first); settings.setValue("repeat",repeat);
            sounds=draft;
            for(size_t i=0;i<sounds.size();++i) {
                QString key=QString::fromUtf8(keys[i]); settings.setValue(key+"/path",sounds[i].path);
                settings.setValue(key+"/volume",sounds[i].volume); settings.setValue(key+"/enabled",sounds[i].enabled);
            }
            language=languages->currentData().toString(); settings.setValue("language",language); loadLanguage();
            panelEnabled=showPanel->isChecked(); settings.setValue("notificationPanel",panelEnabled); updatePanel();
            resetCaptureTimer(); settings.sync();
            if(settings.status()!=QSettings::NoError) QMessageBox::warning(&window,"OBS Companion",trText("Settings could not be saved."));
            tick();
        };
        connect(buttons->button(QDialogButtonBox::Apply),&QPushButton::clicked,&window,applySettings);
        if(window.exec()==QDialog::Accepted) applySettings();
        status=nullptr; monitorStatus=nullptr; pauseButton=nullptr; resumeButton=nullptr; dialogOpen=false;
    }
};
Companion *companion = nullptr;
void event(obs_frontend_event e, void *) {
    if (!companion) return;
    if (e == OBS_FRONTEND_EVENT_EXIT) { companion->shutdown(); return; }
    if (e == OBS_FRONTEND_EVENT_RECORDING_STARTED || e == OBS_FRONTEND_EVENT_RECORDING_STOPPED ||
        e == OBS_FRONTEND_EVENT_RECORDING_PAUSED || e == OBS_FRONTEND_EVENT_RECORDING_UNPAUSED ||
        e == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED || e == OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED)
        QMetaObject::invokeMethod(companion, [] { if (companion) companion->resetCaptureTimer(); }, Qt::QueuedConnection);
    if(e==OBS_FRONTEND_EVENT_SCENE_CHANGED || e==OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED)
        QMetaObject::invokeMethod(companion,[] { if(companion) companion->resetCaptureTimer(); },Qt::QueuedConnection);
    if(e==OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED || e==OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED) {
        const size_t index=e==OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED ? 4 : 5;
        QMetaObject::invokeMethod(companion,[index] { if(companion) companion->play(index); },Qt::QueuedConnection);
    }
    if (e == OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED && companion)
        QMetaObject::invokeMethod(companion, [] { if (companion) companion->play(0); }, Qt::QueuedConnection);
}
void show(void *) { if (companion) companion->dialog(); }
}
bool obs_module_load(void) { return true; }
void obs_module_post_load(void) {
    char *path = obs_module_config_path("settings.ini");
    if (!path) return;
    const QString full = QString::fromUtf8(path); bfree(path);
    QDir().mkpath(QFileInfo(full).absolutePath());
    companion = new Companion(full);
    obs_frontend_add_event_callback(event, nullptr);
    obs_frontend_add_tools_menu_item("OBS Companion", show, nullptr);
}
void obs_module_unload(void) {
    obs_frontend_remove_event_callback(event, nullptr);
    delete companion; companion = nullptr;
}
