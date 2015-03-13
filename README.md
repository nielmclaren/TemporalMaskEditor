TemporalMaskEditor
==================

A tool for editing a temporal mask applied to a short movie clip.

Uses brush code and brush PNGs from Graffiti Research Lab's [L.A.S.E.R. Tag](http://www.graffitiresearchlab.com/blog/projects/laser-tag/) project.

Extract frames from source movie clip.
`ffmpeg -ss 0:00:00 -i "input.mov" -t 10 -f image2 out/frame%04d.png`

Resize frames to 640x480.
`mogrify -path ./ -gravity Center -resize 640x480^ -crop 640x480+0+0 *.png`

Recombine frames into output movie clip.
`ffmpeg -f image2 -start_number 1 -i out/frame%04d.png -vcodec libx264 -sameq output.mov`
