if virtualenv --version ; then
    echo "virtualenv is installed"
else
    pip install virtualenv
fi

if [ ! -d "ENV" ] 
then
    virtualenv ENV
    git clone https://github.com/strictdoc-project/strictdoc && cd strictdoc
    git checkout tags/0.0.30
    python3 setup.py install
    cd .. && rm -rf strictdoc
fi

source ENV/bin/activate

cd ../doc/00_requirements
rm -rf output-html

if [ -f "../../scripts/plantuml.jar" ]; then
    mkdir -p output-html/html/00_requirements
    java -jar ../../scripts/plantuml.jar ../diagrams/appStates.plantuml
    cp ../diagrams/*.png output-html/html/00_requirements
else 
    echo "Please download plantuml.jar to the scripts folder"
fi

strictdoc export . --formats=html --output-dir output-html

cd output-html && python3 -m http.server
cd ../../../scripts
