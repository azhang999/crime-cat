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
	$(DIST)/cat.pnct \
	$(DIST)/cat.scene \
	$(DIST)/cat.boundbox \
	$(DIST)/living_room.pnct \
	$(DIST)/living_room.scene \
	$(DIST)/living_room.boundbox \
	$(DIST)/kitchen.pnct \
	$(DIST)/kitchen.scene \
	$(DIST)/kitchen.boundbox \
	$(DIST)/walls_doors_floors_stairs.pnct \
	$(DIST)/walls_doors_floors_stairs.scene \
	$(DIST)/walls_doors_floors_stairs.boundbox \
	$(DIST)/bedroom.pnct \
	$(DIST)/bedroom.scene \
	$(DIST)/bedroom.boundbox \
	$(DIST)/bathroom.pnct \
	$(DIST)/bathroom.scene \
	$(DIST)/bathroom.boundbox \
	$(DIST)/office.pnct \
	$(DIST)/office.scene \
	$(DIST)/office.boundbox \
	$(DIST)/bounds.pnct \
	$(DIST)/bounds.scene \
	$(DIST)/bounds.boundbox \

$(DIST)/cat.scene : cat.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Player '$@'

$(DIST)/cat.pnct : cat.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Player '$@'

$(DIST)/cat.boundbox : cat.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Player '$@'

$(DIST)/living_room.scene : living_room.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':LivingRoom '$@'

$(DIST)/living_room.pnct : living_room.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':LivingRoom '$@'

$(DIST)/living_room.boundbox : living_room.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':LivingRoom '$@'

$(DIST)/kitchen.scene : kitchen.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Kitchen '$@'

$(DIST)/kitchen.pnct : kitchen.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Kitchen '$@'

$(DIST)/kitchen.boundbox : kitchen.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Kitchen '$@'

$(DIST)/walls_doors_floors_stairs.scene : all_rooms.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':WallsDoorsFloorsStairs '$@'

$(DIST)/walls_doors_floors_stairs.pnct : all_rooms.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':WallsDoorsFloorsStairs '$@'

$(DIST)/walls_doors_floors_stairs.boundbox : all_rooms.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':WallsDoorsFloorsStairs '$@'

$(DIST)/bedroom.scene : bedroom.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Bedroom '$@'

$(DIST)/bedroom.pnct : bedroom.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Bedroom '$@'

$(DIST)/bedroom.boundbox : bedroom.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Bedroom '$@'

$(DIST)/bathroom.scene : bathroom.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Bathroom '$@'

$(DIST)/bathroom.pnct : bathroom.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Bathroom '$@'

$(DIST)/bathroom.boundbox : bathroom.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Bathroom '$@'

$(DIST)/office.scene : office.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Office '$@'

$(DIST)/office.pnct : office.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Office '$@'

$(DIST)/office.boundbox : office.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Office '$@'

$(DIST)/bounds.scene : all_rooms.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Bounds '$@'

$(DIST)/bounds.pnct : all_rooms.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Bounds '$@'

$(DIST)/bounds.boundbox : all_rooms.blend $(EXPORT_BOUNDBOX)
	$(BLENDER) --background --python $(EXPORT_BOUNDBOX) -- '$<':Bounds '$@'