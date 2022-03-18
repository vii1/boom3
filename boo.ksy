meta:
  id: boo
  file-extension: boo
  encoding: cp1252

seq:
  - id: identifier
    size: 4
  - id: map
    type: map

enums:
  wall_kind:
    1: wall
    2: hole
    3: portal
    
types:
  map:
    meta:
      endian:
        switch-on: _root.identifier
        cases:
          "[0x42,0x4f,0x4f,0x4d]": be
          "[0x4d,0x4f,0x4f,0x42]": le
    seq:
      - id: version
        type: u4
        valid: 10
      - id: start_x
        type: f4
      - id: start_y
        type: f4
      - id: start_z
        type: f4
      - id: n_clusters
        type: u4
      - id: clusters
        type: cluster
        repeat: expr
        # repeat-expr: 7
        repeat-expr: n_clusters
      - id: n_verts
        type: u4
      - id: vertices
        type: vertex
        repeat: expr
        repeat-expr: n_verts
      - id: palette
        type: rgb
        repeat: expr
        repeat-expr: 256
      - id: n_textures
        type: u4
      - id: textures
        type: texture
        repeat: expr
        repeat-expr: n_textures
    types:
      cluster:
        seq:
          - id: options
            type: options_cluster
          - id: n_sectors
            type: u4
          - id: sectors
            type: sector
            repeat: expr
            repeat-expr: n_sectors
            # repeat-expr: 1
      sector:
        seq:
          - id: options
            type: options_sector
          - id: zfa
            type: f4
          - id: zfb
            type: f4
          - id: zfc
            type: f4
          - id: zca
            type: f4
          - id: zcb
            type: f4
          - id: zcc
            type: f4
          - id: tfloor
            type: u4
          - id: tceiling
            type: u4
          - id: n_lines
            type: u4
          - id: lines
            type: line
            repeat: expr
            repeat-expr: n_lines
            # repeat-expr: 1
      line:
        seq:
          - id: options
            type: options_line
          - id: v1
            type: u4
          - id: v2
            type: u4
          - id: n_walls
            type: u4
          - id: walls
            type: wall
            repeat: expr
            repeat-expr: n_walls
            # repeat-expr: 1
      wall:
        seq:
          - id: kind
            type: u1
            enum: wall_kind
          - id: options
            type: options_wall
          - id: texture
            type: u4
          - id: z1c
            type: f4
          - id: z2c
            type: f4
          - id: sector
            type: u4
            if: kind != wall_kind::wall
          - id: target
            type: u4
            if: kind == wall_kind::portal
          
      options_cluster:
        seq:
          - id: dirty
            type: b1
          - id: a0
            type: b5
          - id: visited
            type: b1
          - id: a1
            type: b1
      options_sector:
        seq:
          - id: dirty
            type: b1
          - id: a0
            type: b6
          - id: last_sector
            type: b1
      options_line:
        seq:
          - id: dirty
            type: b1
          - id: a0
            type: b6
          - id: contour_end
            type: b1
      options_wall:
        seq:
          - id: dirty
            type: b1
          - id: a0
            type: b5
          - id: portal
            type: b1
          - id: solid
            type: b1
      vertex:
        seq:
          - id: x
            type: f4
          - id: y
            type: f4
      rgb:
        seq:
          - id: r
            type: u1
          - id: g
            type: u1
          - id: b
            type: u1
      texture:
        seq:
          - id: width
            type: u4
          - id: height
            type: u4
          - id: name
            type: str
            size: 32
          - id: data
            size: width * height