Hotel Manager
===========================
Run with 
```
make
```

Clean with 
```
make clean
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

------>  image1.png


Utility methods contain serialization and dumping ,
Printing the server response happens on each server receipt.


------>  image2.png




How the server handles multiple clients,
The server handles multiple clients using cpp threads, when the server accepts a client, it creates and stores the designated FDS and uses them for later , 
Handle-client function is used for this affair


------>  image3.png

Call back functions 



------>  image4.png


Server.h 
Gives basic communication utility with the client , deserializes and serializes the messages in between with it’s functions .



------>  image5.png




Mutex locks are used for maintaining thread safety with the json files that are altered 
,
JSON storage is updated after each insertion, functions that need to be thread safe because of race conditions handle races by acquring mutex locks 


------>  image6.png




JSON utilities enabled with nlohmann library and  hand written Date utility for working with dates


------>  image7.png

------>  image8.png

------>  image9.png

------>  image10.png



Hotel managment logic :


------>  image11.png


1- Overlap occupations finds out the capacity of each room for a potential reservation

2- Authenticate checks the credential user password for entry 

3- Invalid date utility is an interface  that recognizes faults 

4- update_reserves_with_time() , updates the reserves on each call back execution

5- Is User admin checks if the user is admin 




A sample of a call back function that creates response ( each executing thread for the client calls this function )


------>  image12.png

------>  image13.png

------>  image14.png



Outputs :
=======================================

Run server :

------>  image15.png



Run client : 

------>  image16.png



successful login :

------>  image17.png




Login failed :

------>  image18.png




Logout : 

------>  image19.png




View‬‬ ‫‪user‬‬ ‫‪information :

------>  image20.png



‫‪‬‬ 
View‬‬ ‫‪all‬‬ ‫‪users :

------>  image21.png




View‬‬ ‫‪Rooms‬‬ ‫‪Information :

------>  image22.png




Leaving‬‬ ‫‪room :

------>  image23.png




Successful signup : 

------>  image24.png




Error 403 : 

------>  image25.png




Error 503: 

------>  image26.png




Successful ‫‪Booking‬‬ :

------>  image27.png




Add Room : 

------>  image28.png




Modify Room : 

------>  image29.png




Remove Room : 

------>  image30.png




Edit‬‬ ‫‪information :

------>  image31.png




Canceling :

------>  image32.png




Error 451 : 

------>  image33.png




Error 101 : 

------>  image34.png




Error 108 : 

------>  image35.png




Error 111 : 

------>  image36.png




Error 102 : 

------>  image37.png




Error 231 : 

------>  image38.png




Error 109 : 

------>  image39.png




Error 311 : 

------>  image40.png




View Empty room information : 

------>  image41.png


Error 102 (leaving room): 

------>  image42.png
























