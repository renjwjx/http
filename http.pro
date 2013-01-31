HEADERS += httpwindow.h
SOURCES += httpwindow.cpp \
           main.cpp
FORMS += authenticationdialog.ui
QT += network
#CONFIG += console
# install
target.path = $$[QT_INSTALL_EXAMPLES]/network/http
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS http.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/network/http
INSTALLS += target sources

symbian: {
    TARGET.CAPABILITY = NetworkServices
    include($$PWD/../../symbianpkgrules.pri)
}
maemo5: include($$PWD/../../maemo5pkgrules.pri)
contains(MEEGO_EDITION,harmattan): include($$PWD/../../harmattanpkgrules.pri)

