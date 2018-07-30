TEMPLATE = subdirs
TARGET = audiooutput

QT += multimedia widgets

SUBDIRS       = libqtmixer \
                example

libqtmixer.subdir = qtmixer

example.subdir = example
example.depends = libqtmixer

