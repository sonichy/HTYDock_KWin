s="[Desktop Entry]\nName=海天鹰Dock\nComment=HTY Dock\nExec=`pwd`/HTYDock\nIcon=`pwd`/launcher.png\nPath=`pwd`\nType=Application\nTerminal=false\nCategories=System;"
echo -e $s > HTYDock.desktop
cp `pwd`/HTYDock.desktop ~/.local/share/applications/HTYDock.desktop