# Puma: a microservice of extruded visualization (VTK-m) for fusion tokamak simulation (WDM)
## Installation
The easiest way to install Puma dependencies is to use [Spack](www.spack.io). We'll start there. First install spack.
```
git clone https://github.com/m-kim/wdmapp-spack
```
### Note: this is not the standard spack. How come?
Well, it's a fork of a fork. In other words, wdmapp has it's own spack repository, and Puma needed some customizations as well. What kind of customizations? Well, to keep VTK-m compiling smoothly, we opted to leave out the ```CELLSET_EXTRUDE``` from the ```CellSetList.``` in the ```Policy.``` For now, that means there needs to be a separate Puma repository at:
```
git clone https://gitlab.kitware.com/m-kim/vtk-m
``` 

on branch ```extrude-truncate.``` Don't worry though, ```Policy``` is getting a clean up after version 1.6, which should remove this obstacle. But, for now this is where things are. However, the custom spack fork (of a fork) will take care of that. 

### Spack is installed. Now what?
A few things need to be installed. First, activate spack:

```
. ~/wdm-spack/share/spack.setup.sh
```
Next, effis (and more importantly kittie) need to be installed. 

```
spack install effis@develop
```

There are a couple things that I should note. One, OpenMPI seems to be having problems with CMake right now. So, I usually like to install using ```^mpich``` but that's up to user discretion. Second, *running effis requires codar-cheetah.* It is not listed as running requirement, so will need to be installed by hand.

```
spack install codar-cheetah
```

There's a problem with that as well, because codar-cheetah needs python3. This can be problematic if you have code that needs python2 or running on rhea because it doesn't have python3. So, I like (and [Eric](https://github.com/suchyta1/)) like to install local version of both pythons. I'll leave that as an exercise to the reader. 

Once those are installed, install the customized VTK-m:
```
spack install mvtkm
```

### Puma Installation
Now that all the pre-requisites are setup, let's install Puma:
```
git clone git@github.com:m-kim/Puma.git
```
and build it
```
spack load codar-cheetah
spack load effis
spack load mvtkm
cd Puma
mkdir build
cd build
cmake ../
make -j
```

## Running
Congratulations, you've installed Puma. How do you run it? That's a great question, there are two example files in the ```effis-examples``` directory to help you out. The first one is called, simply enough, ```example.yaml.``` I used this one to test at home using data generated on sdg-tm76. To use effis, first the ```example.yaml``` needs to be "composed." This sets up the directories and cheetah to run. 
```
effis-compose.py example.yaml
```

This will create directories in the *cwd* ```runs/run-1``` and pre-populate scripts for executing through cheetah. To run, the composed example needs to be "submitted."
```
effis-submit runs/run-1
```

Note, the directory that was created in the ```compose``` stage is now passed as *cli* to ```effis-submit.``` This will launch Puma in the background.

## example.yaml
Most of this should be explained in the effis documentation. Specifically for Puma, Puma needs to file names and path names. These should be passed in like this:

```
executable_path: /home/mark/Projects/Puma/build/wsl/Debug/pumacli/pumacli
    commandline_args: 
    - -meshname
    - xgc.mesh
    - -meshpathname
    - /mnt/c/Users/mark/Desktop/xgc-gene/
    - -diagname
    - xgc.oneddiag
    - -diagpathname
    - /mnt/c/Users/mark/Desktop/xgc-gene/
    - -filename
    - xgc.3d.00015
    - -filepathname
    - /mnt/c/Users/mark/Desktop/xgc-gene/
```
Note, the ```-meshname,``` ```-diagname``` and ```-filename``` variables do not end in ```.bp.``` 

## Output
Currently, the output is ```output-#.pnm``` i.e. pnm files. 

### Can I run Puma by itself?
No. Although Puma takes all it's files and paths in through the *cli*, it depends on kittie infrastructure to run. Kittie sets up a lot of bash variables in the background to ensure the workflow is composed correctly. It cannot be run by itself.

### No really, can I run Puma myself?
Yes, by going into the code and switching out the kittie for ADIOS calls.

# Conclusion
