all: build 

build: ; cd ./bucketsort/ ; make ; cd .. ; cd ./counter_dir/ ; make     
            
clean: ; rm -rf mysort counter ; cd ./bucketsort ; make clean ; cd .. ; cd ./counter_dir/ ; make clean 