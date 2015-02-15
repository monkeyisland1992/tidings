# The name of your app.
# NOTICE: name defined in TARGET has a corresponding QML filename.
#         If name defined in TARGET is changed, following needs to be
#         done to match new name:
#         - corresponding QML filename must be changed
#         - desktop icon filename must be changed
#         - desktop filename must be changed
#         - icon definition filename in desktop file must be changed
TARGET = harbour-tidings

CONFIG += sailfishapp
QT += concurrent xml

SOURCES += \
    src/harbour-tidings.cpp \
    src/feedloader.cpp \
    src/newsblendmodel.cpp \
    src/htmlfilter.cpp \
    src/urlloader.cpp \
    src/htmlsed.cpp

OTHER_FILES += qml/harbour-tidings.qml \
    qml/cover/CoverPage.qml \
    rpm/harbour-tidings.spec \
    rpm/harbour-tidings.yaml \
    qml/pages/FeedsPage.qml \
    qml/pages/RssModel.qml \
    qml/pages/SourcesPage.qml \
    qml/pages/AtomModel.qml \
    qml/pages/OpmlModel.qml \
    qml/pages/ViewPage.qml \
    qml/pages/WebPage.qml \
    qml/pages/FavIcon.qml \
    qml/pages/favicon.js \
    qml/pages/database.js \
    qml/pages/SourcesModel.qml \
    qml/pages/SourceEditDialog.qml \
    qml/pages/NewsBlendModel.qml \
    qml/pages/AboutPage.qml \
    qml/pages/LicensePage.qml \
    qml/pages/license.js \
    qml/tidings.png \
    harbour-tidings.desktop \
    qml/pages/FancyScroller.qml \
    qml/pages/Notification.qml \
    qml/pages/RdfModel.qml \
    qml/pages/RescalingRichText.qml \
    qml/pages/ExternalLinkDialog.qml \
    qml/pages/FeedSorter.qml \
    qml/pages/SortSelectorPage.qml \
    qml/cover/overlay.png \
    qml/pages/ConfigValue.qml \
    qml/pages/BackgroundWorker.qml \
    qml/pages/FeedStats.qml \
    qml/pages/FeedItem.qml \
    qml/pages/SplashPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/MediaItem.qml \
    qml/pages/ImagePage.qml \
    qml/pages/ResourcesPage.qml \
    qml/pages/placeholder.png

CONFIG += sailfishapp_i18n
TRANSLATIONS += translations/harbour-tidings-ru_RU.ts
TRANSLATIONS += translations/harbour-tidings-fr_FR.ts

CONFIG += sailfishapp_i18n_unfinished

HEADERS += \
    src/feedloader.h \
    src/appversion.h \
    src/json.h \
    src/newsblendmodel.h \
    src/htmlfilter.h \
    src/dateparser.h \
    src/urlloader.h \
    src/htmlsed.h
