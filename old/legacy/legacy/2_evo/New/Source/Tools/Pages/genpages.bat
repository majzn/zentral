call soypage -i downloads.txt -o downloads.html -s style.soy -t template.soy
call soypage -i gallery.txt -o gallery.html -s style.soy -t template.soy
call soypage -i home.txt -o index.html -s style.soy -t template.soy
call soypage -i home.txt -o home.php -s style.soy -t template.soy

call soypage -i downloads.txt -o downloadsdark.html -s styledark.soy -t templatedark.soy
call soypage -i gallery.txt -o gallerydark.html -s styledark.soy -t templatedark.soy
call soypage -i home.txt -o homedark.html -s styledark.soy -t templatedark.soy
call soypage -i home.txt -o homedark.php -s styledark.soy -t templatedark.soy