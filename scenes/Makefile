.PHONY : all

#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	BLENDER = /Applications/Blender.app/Contents/MacOS/Blender -y
else ifeq ($(UNAME_S),Linux)
	BLENDER = ../../blender-2.93.4-linux-x64/blender -y
else
	BLENDER = blender
endif

EXPORT_MESHES=export-meshes.py
EXPORT_SCENE=export-scene.py
EXPORT_BOUNDBOX=export-boundbox.py

DIST=../dist

all : \
	$(DIST)/player.pnct \
	$(DIST)/player.scene \
	$(DIST)/player.boundbox \
	$(DIST)/living_room.pnct \
	$(DIST)/living_room.scene \
	$(DIST)/living_room.boundbox \
	$(DIST)/kitchen.pnct \
	$(DIST)/kitchen.scene \
	$(DIST)/kitchen.boundbox \

$(DIST)/player.scene : living_room.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Player '$@'

$(DIST)/player.pnct : living_room.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Player '$@'

$(DIST)/player.boundbox : living_room.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Player '$@'

$(DIST)/living_room.scene : living_room.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':LivingRoom '$@'

$(DIST)/living_room.pnct : living_room.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':LivingRoom '$@'

$(DIST)/living_room.boundbox : living_room.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':LivingRoom '$@'

$(DIST)/kitchen.scene : living_room.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Kitchen '$@'

$(DIST)/kitchen.pnct : living_room.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Kitchen '$@'

$(DIST)/kitchen.boundbox : living_room.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Kitchen '$@'