#!/usr/bin/env python

#based on 'export-meshes.py' from base 3 code.
#Patched for 15-466-f19 to remove non-pnct formats!
#Patched for 15-466-f20 to merge data all at once (slightly faster)

#Note: Script meant to be executed within blender 2.9, as per:
#blender --background --python export-boundbox.py -- [...see below...]

import sys,re
import mathutils
from mathutils import Vector

args = []
for i in range(0,len(sys.argv)):
    if sys.argv[i] == '--':
        args = sys.argv[i+1:]

if len(args) != 2:
    print("\n\nUsage:\nblender --background --python export-meshes.py -- <infile.blend[:collection]> <outfile.boundbox>\nExports the meshes referenced by all objects in the specified collection(s) (default: all objects) to a binary blob.\n")
    exit(1)

import bpy

infile = args[0]
collection_name = None
m = re.match(r'^(.*):([^:]+)$', infile)
if m:
    infile = m.group(1)
    collection_name = m.group(2)
outfile = args[1]

assert outfile.endswith(".boundbox")

print("Will export meshes referenced from ",end="")
if collection_name:
    print("collection '" + collection_name + "'",end="")
else:
    print('master collection',end="")
print(" of '" + infile + "' to '" + outfile + "'.")

import struct

bpy.ops.wm.open_mainfile(filepath=infile)

if collection_name:
    if not collection_name in bpy.data.collections:
        print("ERROR: Collection '" + collection_name + "' does not exist in scene.")
        exit(1)
    collection = bpy.data.collections[collection_name]
else:
    collection = bpy.context.scene.collection


#meshes to write:
to_write = set()
did_collections = set()
def add_meshes(from_collection):
    global to_write
    global did_collections
    if from_collection in did_collections:
        return
    did_collections.add(from_collection)

    if from_collection.name[0] == '_':
        print("Skipping collection '" + from_collection.name + "' because its name starts with an underscore.")
        return

    for obj in from_collection.objects:
        if obj.type == 'MESH':
            if obj.data.name[0] == '_':
                print("Skipping mesh '" + obj.data.name + "' because its name starts with an underscore.")
            else:
                to_write.add(obj.data)
        if obj.instance_collection:
            add_meshes(obj.instance_collection)
    for child in from_collection.children:
        add_meshes(child)

add_meshes(collection)
#print("Added meshes from: ", did_collections)

#set all collections visible: (so that meshes can be selected for triangulation)
def set_visible(layer_collection):
    layer_collection.exclude = False
    layer_collection.hide_viewport = False
    layer_collection.collection.hide_viewport = False
    for child in layer_collection.children:
        set_visible(child)

set_visible(bpy.context.view_layer.layer_collection)

#data contains 8 bounding box coords from the meshes:
data = []

#strings contains the mesh names:
strings = b''

#index gives offsets into the data (and names) for each mesh:
index = b''

obj_count = 0
for obj in bpy.data.objects:
    if obj.data in to_write:
        to_write.remove(obj.data)
    else:
        continue

    obj.hide_select = False
    mesh = obj.data
    name = mesh.name

    print("Writing '" + name + "'...")

    if bpy.context.object:
        bpy.ops.object.mode_set(mode='OBJECT') #get out of edit mode (just in case)

    #select the object and make it the active object:
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    
    # SOURCE: https://blender.stackexchange.com/questions/8459/get-blender-x-y-z-and-bounding-box-with-script
    bbox_corners = [bpy.context.object.matrix_world @ Vector(corner) for corner in bpy.context.object.bound_box]

    #record mesh name, start position and vertex count in the index:
    name_begin = len(strings)
    strings += bytes(name, "utf8")
    name_end = len(strings)
    index += struct.pack('I', name_begin)
    index += struct.pack('I', name_end)

    local_data = b''
    for p in bbox_corners:
        local_data += struct.pack('fff', p[0], p[1], p[2])

    data.append(local_data)

    obj_count += 1

data = b''.join(data)

#check that code created as much data as anticipated:
assert(obj_count * (4*3*8) == len(data))

#write the data chunk and index chunk to an output blob:
blob = open(outfile, 'wb')
#first chunk: the data
blob.write(struct.pack('4s',b'pnct')) #type
blob.write(struct.pack('I', len(data))) #length
blob.write(data)
#second chunk: the strings
blob.write(struct.pack('4s',b'str0')) #type
blob.write(struct.pack('I', len(strings))) #length
blob.write(strings)
#third chunk: the index
blob.write(struct.pack('4s',b'idx0')) #type
blob.write(struct.pack('I', len(index))) #length
blob.write(index)
wrote = blob.tell()
blob.close()

print("Wrote " + str(wrote) + " bytes [== " + str(len(data)+8) + " bytes of data + " + str(len(strings)+8) + " bytes of strings + " + str(len(index)+8) + " bytes of index] to '" + outfile + "'")
