Hotel Manager
===========================
This is a hotel manager , implemented in CPP. 
Some of the available commands and their schemas
### Signup 
```
signup <username to be checked >
```

### Login
```
login <username> <password>
```
### Logout 
```
9
```
### Book Room  
```
book_room <room_id> <bed_count> <start_date> <end_date>
```
### Cancel Room
```
cancel <room_id> <bed_count>
```
Build  with 
```
make
```

Clean with 
```
make clean
```
Run Client with 
```
./Client.exe
```
Run server with 
```
./Server.exe
```

```
Command directives are in the PDF of the proejct file 

```
810199395 Pouriya Tajmehrabi
810199392 Mohammad Amin Pourzare


معماری معماری Client server است .
فرمت اطلاعات که بین نود های سیستم منتقل می شود json است
کلاینت با استفاده از request-type در هر درخواستی که به سرور میفرستد مخابره می کند که از کدام یک از method ها زیر استفاده می خواهد بکند.


یک Wrapper دور Socket زده شده است که Connection با سرور را Manage می کند

کلاس Connector  در کلاینت.
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image1.png)
------>  image1.png


Utility methods contain serialization and dumping ,
Printing the server response happens on each server receipt.


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image2.png)




How the server handles multiple clients,
The server handles multiple clients using cpp threads, when the server accepts a client, it creates and stores the designated FDS and uses them for later , 
Handle-client function is used for this affair


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image3.png)


Call back functions 



![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image4.png)



Server.h 
Gives basic communication utility with the client , deserializes and serializes the messages in between with it’s functions .



![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image5.png)





Mutex locks are used for maintaining thread safety with the json files that are altered 
,
JSON storage is updated after each insertion, functions that need to be thread safe because of race conditions handle races by acquring mutex locks 


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image6.png)





JSON utilities enabled with nlohmann library and  hand written Date utility for working with dates


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image7.png)
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image8.png)
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image9.png)
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image10.png)



Hotel managment logic :


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image11.png)


1- Overlap occupations finds out the capacity of each room for a potential reservation

2- Authenticate checks the credential user password for entry 

3- Invalid date utility is an interface  that recognizes faults 

4- update_reserves_with_time() , updates the reserves on each call back execution

5- Is User admin checks if the user is admin 




A sample of a call back function that creates response ( each executing thread for the client calls this function )


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image12.png)


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image13.png)


![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image14.png)




Outputs :
=======================================

Run server :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image15.png)




Run client : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image16.png)



successful login :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image17.png)





Login failed :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image18.png)





Logout : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image19.png)





View‬‬ ‫‪user‬‬ ‫‪information :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image20.png)




‫‪‬‬ 
View‬‬ ‫‪all‬‬ ‫‪users :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image21.png)





View‬‬ ‫‪Rooms‬‬ ‫‪Information :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image22.png)





Leaving‬‬ ‫‪room :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image23.png)





Successful signup : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image24.png)





Error 403 : 
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image25.png)





Error 503: 
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image26.png)





Successful ‫‪Booking‬‬ :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image27.png)





Add Room : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image28.png)





Modify Room : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image29.png)





Remove Room : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image30.png)





Edit‬‬ ‫‪information :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image31.png)





Canceling :

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image32.png)





Error 451 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image33.png)





Error 101 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image34.png)




Error 108 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image35.png)




Error 111 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image36.png)





Error 102 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image37.png)





Error 231 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image38.png)





Error 109 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image39.png)





Error 311 : 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image40.png)





View Empty room information : 
![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image41.png)



Error 102 (leaving room): 

![alt text](https://github.com/SyntheticDemon/CN_CHomeworks_1/blob/main/docs/images/image42.png)

























