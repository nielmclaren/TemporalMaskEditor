TemporalMaskEditor
==================

A tool for editing a temporal mask applied to a short movie clip.

Uses brush code and brush PNGs from Graffiti Research Lab's [L.A.S.E.R. Tag](http://www.graffitiresearchlab.com/blog/projects/laser-tag/) project.

Use these ffmpeg commands to extract frames as images and then recombine them into a movie clip:

`ffmpeg -ss 0:00:00 -i "input.mov" -t 10 -sameq -f image2 out/frame%04d.jpg`

`ffmpeg -f image2 -start_number 1 -i out/frame%04d.jpg -vcodec libx264 -sameq output.mov`
