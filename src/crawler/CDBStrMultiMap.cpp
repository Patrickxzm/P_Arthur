
#include <iostream>
#include"CDBStrMultiMap.h"

#if !defined(DB_VERSION_MAJOR) || !defined(DB_VERSION_MINOR)
#error "Berkerly db not found in your system, abort..."
#endif

#if DB_VERSION_MAJOR < 4
#error "Berkerly db version not matched, need Ver4.0 at least, abort..."
#else

#if DB_VERSION_MINOR == 0
#define _BDBTXNARG_ 
#define _BDBTXNARG_1
#elif DB_VERSION_MINOR <= 2
#define _BDBTXNARG_  NULL,
#define _BDBTXNARG_1
#elif DB_VERSION_MINOR == 3
#define _BDBTXNARG_ NULL,
#define _BDBTXNARG_1 NULL,
#endif

#endif



CDBStrMultiMap::CDBStrMultiMap()
{
	db = new Db(0,0);
	db_is_open = false;
}

bool CDBStrMultiMap::open( const char* name, int flags )
{
	
	db->set_flags(DB_DUP);
	
	int ret;
	
	if( flags == 1){
		try{
			ret = db->open( _BDBTXNARG_  name, NULL , DB_BTREE , DB_CREATE , 0664 );
			if ( ret!= 0 ) return false;
		}catch( DbException &e)
		{
			cerr<<"!! Exception in open the DB : "<< e.what() <<"\n";
			return false;
		}
	}
	else{
		if( flags == 2 )
		{
			try{
			
				ret = db->open( _BDBTXNARG_  name, NULL , DB_BTREE, DB_TRUNCATE ,0664 );
				if ( ret!= 0) return false;
			}
			catch( DbException &e)
			{
				cerr<<"!! Exception in open the DB : "<< e.what()<<endl;
				return false;
			}	
			
		}
		else{
			if( flags == 0)
			{
        	                try{  
					ret = db->open( _BDBTXNARG_  name, NULL , DB_BTREE, 0 ,0664 );		            
					if ( ret!= 0) return false;
				                        
				}
                	        catch( DbException &e)	                       
		       		{		                             
				      	cerr<<"!! Exception in open the DB : "<< e.what()<<endl;			 
 					return false;				                       
			       	}
			
			}
			else{
				if( flags == 3 )
				{
					try{
						ret = db->open( _BDBTXNARG_  name, NULL , DB_BTREE , DB_CREATE | DB_TRUNCATE , 0664 );
						if( ret!= 0) return false;
						
					}
					catch( DbException &e)
					{
						cerr<<"!! Exception in open the DB : "<< e.what() <<endl;
						return false;
					}
				}
				else{
					cout<<"==== wrong flags."<<endl;
					return false;
				}
			}
		}
	}
	db_is_open = true;
	return true;
}

bool CDBStrMultiMap::put( const string &key,  vector<string> &values)
{		
	if( db_is_open == false ) 
	{
		cout<<"==== Can not put items into BDB.  BDB has not been opened."<<endl;
		return false;
	}
	
	
	char  key1[1024];
	memset(key1, 0, 1024);
       	strcpy(key1, key.c_str());
	Dbt dbt_key( key1, key.length() +1 );

	/*
	char* key1 = (char*)key.c_str();
	Dbt dbt_key( key1, strlen(*key1) +1 );
	*/


	char data1[4096];
	cout<<endl;
	
	vector<string>::iterator temp_iter = values.begin();
	for( ; temp_iter != values.end(); temp_iter++ )
	{
		string data_str = *temp_iter;
		
		memset(data1, 0, 4096);
		strcpy(data1, data_str.c_str());
		Dbt dbt_data( data1, data_str.length()+1 );
		
		try{		
			int ret = db->put( 0, &dbt_key, &dbt_data, 0);
			if(ret!=0){
				cout<<"==== Put failed! ret: "<<ret<<endl;
				return false;
			}
		}
		catch( DbException &e)
		{
			cout<<"!! Exception in putL "<<e.what()<<endl;
			return false;
		}
				
	}
	return true;
}

bool CDBStrMultiMap::get( const string &key, vector<string> &values )
{
        if( db_is_open == false )       		        
	{               				        
		cout<<"==== Can not get items from BDB.BDB has not been opened."<<endl;      	
		return false;                                                                     
	}

	
	char  key1[1024];
	memset(key1, 0, 1024);
        strcpy(key1, key.c_str());
        Dbt dbt_key( key1, key.length() +1 );

	try{
		Dbc *dbcp;	
		db->cursor(NULL, &dbcp, 0);
		
		Dbt data;
		while( dbcp->get(&dbt_key, &data, DB_SET ) == 0){
			char *data_string = (char*)data.get_data();
			values.push_back( data_string );
			dbcp->del(0);
		}
		dbcp->close();
	}
	catch( DbException &e)
	{
		cerr<<"!! Exception: "<<e.what()<<endl;
		return false;
	}	
			
	return true;
	
}

bool CDBStrMultiMap::get_any( string &key , vector<string> &values )
{
        if( db_is_open == false )
        {
                cout<<"==== Can not get items from BDB.BDB has not been opened."<<endl;
                return false;
        }
	try{	
		Dbc *dbcq;		
		db->cursor(NULL, &dbcq, 0);
			
		Dbt dbt_key1;
		Dbt dbt_data1;
		char* key_str;
		char* data_str;
		
		if( dbcq->get(&dbt_key1, &dbt_data1, DB_FIRST) == 0 )
		{	
			key_str = (char*)dbt_key1.get_data();
			
			for( ; key_str< key_str + strlen(key_str) ; key_str++ )
			{
				key += *key_str;
			}
			
			data_str = (char*)dbt_data1.get_data();
			values.push_back( data_str );

			dbcq->del(0);
			//Dbt dbt_key2( *key_str , strlen(key_str) );
			
			while( dbcq -> get(&dbt_key1, &dbt_data1, DB_SET) == 0 )
			{
				key_str = (char*)dbt_key1.get_data();
				values.push_back( data_str );
				dbcq->del(0);
			}	
		}
		dbcq->close();
	}
	catch( DbException &e)
	{
		cerr<<"!! Exception: "<<e.what()<<endl;
		return false;
	}
	return true;
}

bool CDBStrMultiMap::get_index( vector<string> &indexvec )
{
	if( db_is_open == false )
	{
		cout<<"==== Can not get items from BDB.BDB has not been opened."<<endl;
		return false;
	}
	try{
		Dbc *dbcp;
		db->cursor(NULL, &dbcp, 0);
		
		Dbt dbt_key1;
		Dbt dbt_data1;
		char* key_str;
		//char* data_str;

		string key;
		
		while( dbcp->get(&dbt_key1, &dbt_data1, DB_NEXT ) ==0 )
		{
			key_str = (char*)dbt_key1.get_data();

			//get the key
			for( ; key_str < key_str + strlen(key_str) ; key_str++ )
			{
				key += *key_str;
			}

			if( find( indexvec.begin(), indexvec.end(),  key ) == indexvec.end() ){  //if it is not a duplicate key
				indexvec.push_back( key );
			}
			key.clear();
			//dbcp->del(0);
		}
		dbcp->close();
	}
	catch( DbException &e)
	{
		cerr<<"!! Exception: "<<e.what()<<endl;
		return false;
	}
	return true;

}


CDBStrMultiMap::~CDBStrMultiMap()
{
	if( db_is_open == true ){
		db->close(0);
		db_is_open = false;
	}
	delete(db);
}
	
bool CDBStrMultiMap::del( const string &key  )
{
		
	if( db_is_open == false )
	{				
		cout<<"====Can not put items into BDB.BDB has not been opened."<<endl;	
		return false;

	}	
	
	char key1[1024];
	memset(key1, 0, 1024);
	strcpy(key1, key.c_str());
	
	Dbt dbt_key( key1, strlen(key1)+1 );				

	try{
		int ret = db->del( NULL, &dbt_key , 0);
		if( ret != 0)
		{
			cout<<"==== Delete failed! "<<endl;
			return false;
		}
	}
	catch( DbException &e)
	{
		cerr<<"!! Delete Error: "<< e.what() <<endl;
		return false;
	}
	cout<<"==== Delete "<<key<<" successfully. "<<endl;
	return false;
	
}

bool CDBStrMultiMap::set_cachesize( int kbytes, int ncache )
{
        int ret;
        try{
               ret = db->set_cachesize( 0, kbytes*1024 , ncache );
        }
        catch( DbException &e)
        {
                return false;
        }
        if ( ret == 0)
                return true;
        else{
                return false;
        }
}

bool CDBStrMultiMap::set_pagesize( int pagesize )
{
        int ret;
        try{
                ret = db->set_pagesize( pagesize );
        }
        catch( DbException &e)
        {
                return false;
        }
        if( ret == 0 )
                return true;
        else{
                return false;
        }
}

int CDBStrMultiMap::get_size()
{
	int size;
	
	DB_BTREE_STAT *db_stat;
	try{
		db->stat(_BDBTXNARG_1 &db_stat, 0 );
		size = (u_long)db_stat->bt_ndata;
	}
	catch( DbException &e )
	{
		cerr<<"!! Exception in get_zise:  "<<e.what()<<endl;
	}
	free( db_stat );
	return size;
}

int CDBStrMultiMap::get_numof_key()
{
        int size;

        DB_BTREE_STAT *db_stat;
        try{
                db->stat(_BDBTXNARG_1 &db_stat, 0 );
                size = (u_long)db_stat->bt_nkeys;
        }
        catch( DbException &e )
        {
                cerr<<"!! Exception in get_numof_key:  "<<e.what()<<endl;
        }
        free( db_stat );
        return size;
}

void CDBStrMultiMap::close()
{
	if( db_is_open == true ){
		db->close(0);
		db_is_open = false;
	}
}
                           

#ifdef _TEST
/***************************************************************************
                          Main.cpp  -  description
                             -------------------
    begin                : 07/15/2004
    copyright            : (C) 2004 by Vito
    email                : 
 ***************************************************************************/

#include <iostream>
//#include "../BDB/db_cxx.h"
#include "db_cxx.h"

#include "CDBStrMultiMap.h"
#include <vector>

using namespace std;
using std::vector;


int main()
{
	CDBStrMultiMap* DbMap;
	DbMap = new CDBStrMultiMap();
	
	char name[128];
	string flag1,flag2;
			
	
	cout<<"Please input Berkeley DB name > ";
	cout.flush();
	cin >> name ;
	
	cout<<"Set OO_CREAT flag(y/n) > ";
	cin>>flag1;
	while( flag1 != "y" && flag1 !="n" )
	{
		cout<<"==== Invalid input.Please input again "<<endl;
		cin>>flag1;
	}
	cout<<"Set OO_TRUNC flag(y/n) > ";
	cin>>flag2;	       
       	while( flag2 != "y" && flag2 != "n")		
	{				
		cout<<"==== Invalid input.Please input again > (y/n)"<<endl;	
		cin>>flag2;								
	}
	
	bool ret;
	
	if ( flag1 == "y" && flag2 == "y" )
	{
		ret = DbMap->open( name , OO_CREAT | OO_TRUNC );	
		if( !ret ){
			cout<<"==== Can not open the Berkeley DB!"<<endl;	
			return 1;
		}
	}
	else{
	       	if( flag1 == "y" && flag2 == "n")
		{
			ret = DbMap->open( name, OO_CREAT );			               
		       	if( !ret ){						            
		    		cout<<"==== Can not open the Berkeley DB!"<<endl;	
				return 1;
							
			}
		}
		else{
			if( flag1 == "n" && flag2 == "y" )
			{
				ret = DbMap->open( name ,OO_TRUNC );				 
		 		
				if( !ret ){
					cout<<"==== Can not open the Berkeley DB!"<<endl;	
					return 1;
				}
			}
			else{
				ret = DbMap->open( name , 0 );
                		if( !ret ){
                        		cout<<"==== Can not open the Berkeley DB!"<<endl;
                        		return 1;
                		}			
			}
								
		}
	}
	cout<<"==== Berkeley DB opened successfully. "<<endl;
	
	string choose;	       
       	int choose_i;
	string input_key;		    
    	string input_value;			 
 	//vector<string> input_vec;
	string output_key;
	//vector<string> output_vec;
	string output_string;

    while(1){
	
	cout<<"******************************************"<<endl;
	cout<<"* Select operation:                      *"<<endl;
	cout<<"* p)ut: insert a key and some values.    *"<<endl;
	cout<<"* g)et: get a key and all its values.    *"<<endl;
	cout<<"*         (\"$\" to get a key/data pair).  *"<<endl;
	cout<<"* i): get the index of the BDB.          *"<<endl;
	cout<<"* n): get the number of key/data pairs.  *"<<endl;
	cout<<"* k): get the mumber of unique key.      *"<<endl;
	cout<<"* e)xit: close the Berkeley DB and exit. *"<<endl;
	cout<<"******************************************"<<endl;
	cout<<"Input your choose (p/g/i/n/k/e) > ";
	cin>>choose;
	if( choose.length() > 1)
	{
		cout<<"==== Invalid choose! Please choose again. "<<endl;
		continue;
	}
	
	if( choose == "p")choose_i = 0;
	if( choose == "g")choose_i = 1;
	if( choose == "n")choose_i = 2;
	if( choose == "k")choose_i = 3;
	if( choose == "e")choose_i = 4;
	if( choose == "i")choose_i = 5;
	
	bool result;
	vector<string> input_vec;
	vector<string> output_vec;
	vector<string> index_vec;
	int number_of_keydata;
	int number_of_key;
	
	bool is_break = false;
	bool firstdata = true;
	
	switch( choose_i ){
		case 0:
			cout<<"Please input a key > ";
			
			cin>>input_key;
			//input_vec.clear();
						
			cout<<"Input the first value ( press \"$\" if don't want to input a value ) > ";
			
			/*
			cin>>input_value;
			if( input_value == "$" ){
				cout<<"==== Null data.The key will not be put into the DB."<<endl;
				break;
			}
			*/
			

			
			while( cin>>input_value )
			{
                        	if( input_value == "$" ){
					if( firstdata == true ){
                                		cout<<"==== Null data.The key will not be put into the DB."<<endl;
						firstdata = false;
						is_break = true;
					}
					break;
                        	}
				else{				
					input_vec.push_back( input_value );
					cout<<"Input next value ( press \"$\" if finished ) > ";
				}
				firstdata = false;
				
			}
			cin.clear();
			if( is_break == true )break;
			
			//cout<<"In main:  size of input_vec: "<<sizeof(input_vec)<<endl;
			if( !DbMap->put( input_key, input_vec ) )
			{
				//cout<<endl;
				cout<<"==== Insert failed. "<<endl;
			}
			else{
				//cout<<endl;
				cout<<"==== Insert items seccessfully! "<<endl;
			}
			break;
			
		case 1:
			cout<<"Please input a key for search or press \"$\">  " ;
			cin>>output_key;
			if( output_key == "$" )
			{
                       		output_key.clear();
                        	result = DbMap->get_any( output_key, output_vec );
                         	if( !result )
	                        {
		                         cout<<"==== Get values failed. "<<endl;
		                }
 		                else{
					if( output_vec.size() == 0 )       
				 		cout<<"==== The DB is empty. "<<endl;
					else{
                        	                cout<<"==== One key/data pair in the DB:  "<<endl;
                                	        cout<<"Key:  "<<output_key<<endl;
                                        	cout<<"Value:  ";
                                        	vector<string>::iterator iter = output_vec.begin();
                                        	for( ; iter != output_vec.end(); iter++)
	                                        {
		                                           cout<<*iter<<"\t";
		                   		}
                                       		 cout<<endl;
                                	}
                        	}
				cin.clear();				
				break;
			}			
			//output_vec.clear();
			
			//cout<<"Output: "<<output_key<<endl;
			
			result = DbMap->get( output_key,output_vec );
			if( !result)
			{
				cout<<"==== Get values failed. " <<endl;
			}
			else{
				if( output_vec.size() == 0 )
					cout<<"==== No key or no value. "<<endl;
				else{
					cout<<"==== Values associated with this key is: "<<endl;
					vector<string>::iterator iter = output_vec.begin();
					for( ; iter != output_vec.end(); iter++ )
						cout<<*iter<<endl;
						
				}
			}
			cin.clear();
			break;
		case 2:
			number_of_keydata = DbMap->get_size();
			cout<<"==== The number of key/data pairs in the database is:   "<< number_of_keydata <<endl;	
			cin.clear();
			break;
		case 3:
			number_of_key = DbMap->get_numof_key();
			cout<<"==== The number of unique key in the database is:   "<< number_of_key <<endl;
			cin.clear();
			break;
			
		case 4:
			DbMap->close();
			cin.clear();
			exit(0);	
		case 5:
			index_vec.clear();
			DbMap->get_index( index_vec );
			if( index_vec.size() == 0 )
				cout<<"==== No key/data. "<<endl;
			else{
				cout<<"==== index nums : "<< index_vec.size() <<endl;
				cout<<"==== All the keys are as follows: "<<endl;
				vector<string>::iterator iter = index_vec.begin();
				for( ; iter != index_vec.end(); iter++ )
					cout<<*iter<<"\t";
			}
			cout<<"\n";
			cin.clear();
			break;
			
		default:
			cout<<"==== Invalid input! "<<endl;
			cin.clear();
			break;
			
	}
	cin.clear();
    }


    	delete(DbMap);
	return 0;
}

#endif // _TEST
