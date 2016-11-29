/* Fibonacci series DP TopDown
*/
#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
int getMinChanges(string s1,string s2,int n , int m)
{

	int ret1,ret2,ret3;
 	if( n==0)return m;
    	if( m==0 )return n;
	if( s1[n]==s2[m])
		return getMinChanges(s1,s2,n-1,m-1);
	else 
		return min(min(1+getMinChanges(s1,s2,n,m-1),1+getMinChanges(s1,s2,n-1,m)),1+getMinChanges(s1,s2,n-1,m-1));
}
unsigned long long int coinCount(int coins[],int amount,int size)
{
	unsigned long long int arr[amount+1][size];
	for(int i=0;i<amount+1;i++)
		for(int j=0;j<size;j++)
			arr[i][j]=1;
    for(int i=1 ;i<=amount;i++)
	   for(int j=0;j<size;j++)
		{
			unsigned long long int x=((i-coins[j])>=0?arr[i-coins[j]][j]:0);
			unsigned long long int y=j>=1?arr[i][j-1]:0;
			arr[i][j]=x+y;
		}
	return arr[amount][size-1];
}
unsigned long long topDownFib(unsigned long long n,unsigned long long arr[])
{   if(arr[n]>0)
        return arr[n];
    else
    {
        if(n==0)
            return 0;
        if(n<=2)
            return 1;
        unsigned long long temp =topDownFib(n-1,arr)+topDownFib(n-2,arr);
        arr[n]=temp;
        return temp;
    }
}
unsigned long long bottomUpFib(unsigned long long n,unsigned long long arr[])
{
    arr[0]=0;arr[1]=1;
    for(unsigned long long i=2;i<=n;i++)
        arr[i]=arr[i-1]+arr[i-2];
    return arr[n];
}
int main() {
    unsigned long long n,tdResult,buResult;
    cin>>n;
    unsigned long long tdarr[n+1]={0};
    unsigned long long buarr[n+1]={0};

    tdResult=topDownFib(n,tdarr);
    buResult=bottomUpFib(n,buarr);
    cout<<tdResult<<" "<<buResult;

}

//delete from here 


package com.xmlparser.parser;

import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.Scanner;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;

import org.apache.commons.lang.StringUtils;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.log4j.Logger;
import org.apache.spark.SparkConf;
import org.apache.spark.api.java.JavaPairRDD;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function;
import org.apache.spark.api.java.function.PairFunction;
import org.apache.spark.api.java.function.VoidFunction;
import org.apache.spark.sql.DataFrame;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.hive.HiveContext;
import org.json.simple.JSONObject;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import scala.Tuple2;

import com.xmlparser.model.DocBean;

public class BuildWordDocMail {
	final static Logger LOGGER = Logger.getLogger(ByteArrayInputStream.class);

	final static SparkConf SPARKCONF = new SparkConf().setAppName(
			"BuildWordDocChat").setMaster("local");
	static FileSystem fs = null;
	static Configuration conf = new Configuration();
	public static Properties prop = new Properties();
	static InputStream input = null;
	static String conf_file = null;
	static Scanner in = null;
	static String hdfsInputPath = "";
	static Path docPath = null;
	static Map<String, String> user_info = new HashMap<String, String>();

	public static void main(String args[]) {

		final JavaSparkContext JAVA_CONTEXT = new JavaSparkContext(SPARKCONF);
		final HiveContext HIVE_CONTEXT = new HiveContext(JAVA_CONTEXT.sc());

		try {

			in = new Scanner(new FileReader(args[0]));
			conf_file = args[1];
			final String xmlSource = args[2].toLowerCase();
			String xmlType = args[3];

			input = new FileInputStream(conf_file);
			prop.load(input);

			String dateFromFile = "";
			while (in.hasNext()) {
				dateFromFile += in.next();
			}

			final String dateArg = dateFromFile.trim().substring(4);
			String[] dateArray = dateArg.split("-");
			in.close();
			hdfsInputPath = prop.getProperty("HDFS_INPUT_PARENT_DIRECTORY")
					+ dateArray[0] + "/" + dateArray[1] + "/" + dateArray[2];

			final Path docPath = new Path(hdfsInputPath + "/" + "Content_"
					+ xmlSource);
			final String documentPath = hdfsInputPath + "/" + "Content_"
					+ xmlSource;
			fs = FileSystem.get(conf);
			if (fs.exists(docPath)) {
				fs.delete(docPath, true);
			}
			fs.mkdirs(docPath);

			JavaPairRDD<String, String> channel_pair_rdd = JAVA_CONTEXT
					.wholeTextFiles(
							hdfsInputPath + "/" + "ReutersSplitXmls");
			String CSVTABLE = xmlSource
					+ prop.getProperty("UNDER_SCORE_SEPARATOR") + xmlType
					+ prop.getProperty("CSV_TABLE_SUFFIX");
			JavaRDD<DocBean> channel_rdd = channel_pair_rdd
					.map(new Function<Tuple2<String, String>, DocBean>() {

						private static final long serialVersionUID = 1L;

						public DocBean call(Tuple2<String, String> arg0)
								throws Exception {
							System.out.println("******arg0._1*******"+arg0._1);
							return new DocBean(arg0._1
									.replace("hdfs://uatnameservice1", ""), arg0._2
									.replace("\n", ""));
						}
					});

			DataFrame Channel_df = HIVE_CONTEXT.createDataFrame(channel_rdd,
					DocBean.class);
			Channel_df.registerTempTable("Channel_df");

			DataFrame channel_final_df = HIVE_CONTEXT
					.sql("select max(a.seq_num)as seq_num,max(b.content) as content from "
							+ prop.getProperty("DATABASE_NAME")
							+ "."
							+ "reuters_chat_csv_temp a join Channel_df b on a.filename=b.filepath group by a.filename");
			channel_final_df.show();
			
		
			
			if (xmlSource.equalsIgnoreCase("Reuters")) 
			{
				JavaPairRDD<String, String> userinfo_rdd = JAVA_CONTEXT.wholeTextFiles(
						hdfsInputPath + "/"        
				//+ StringUtils.capitalize(xmlSource)        
				+ "ReutersUserInfoSplitXmls", 15);

				JavaPairRDD<String, String> user_details = userinfo_rdd.mapToPair(new PairFunction<Tuple2<String, String>, String, String>() {

					private static final long serialVersionUID = 1L;

					public Tuple2<String, String> call(Tuple2<String, String> arg0)   throws Exception {
						DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
						String uIdentifier = null;
						String uFirstName = null;
						String uLastName = null;

						try {
							DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
							Document doc = dBuilder.parse(new InputSource(new ByteArrayInputStream(arg0._2.getBytes(prop.getProperty("CHARSET_UTF_8")))));
							doc.getDocumentElement().normalize();

							XPath xpath = XPathFactory.newInstance().newXPath();

							uIdentifier = xpath.evaluate("/UserInfo/Identifier", doc);
							uFirstName = xpath.evaluate("/UserInfo/FirstName", doc);
							uLastName = xpath.evaluate("/UserInfo/LastName", doc);
							return new Tuple2<String, String>(uIdentifier.toLowerCase(), uFirstName + " "        + uLastName);

						} catch (Exception e) {
							e.printStackTrace();
						}
						return new Tuple2<String, String>(uIdentifier.toLowerCase(), uFirstName + " " + uLastName);
					}
				});
				user_info = user_details.collectAsMap();
		
				channel_final_df.javaRDD().foreach(new VoidFunction<Row>() {

					private static final long serialVersionUID = 1L;

					@SuppressWarnings("unchecked")
					public void call(Row record) throws Exception {

						String docname = record.getString(0);
						String xmlString = record.getString(1);
						JSONObject obj = new JSONObject();
						DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
						DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
						Document doc = dBuilder.parse(new InputSource(new ByteArrayInputStream(xmlString.getBytes("UTF-8"))));
						doc.getDocumentElement().normalize();

						XPath xpath = XPathFactory.newInstance().newXPath();
						NodeList participants = (NodeList) xpath.evaluate("/Chat/Participants/User", doc, XPathConstants.NODESET);
						String participant = "";

						if (participants != null) {
								String user = xpath.evaluate("/Chat/Participants/User[" + 1 + "]", doc);
								participant = user_info.get(user.toLowerCase()) + "<" + user + ">";
								obj.put("Participants", participant);
						}
						
						String eventInfo = "";
						NodeList event = (NodeList) xpath.evaluate("/Chat/Events/Event", doc, XPathConstants.NODESET);
						if (event != null) {
							for (int i = 1; i <= event.getLength(); i++) {
								NodeList eventTypeList = (NodeList) xpath.evaluate("/Chat/Events/Event[" + i + "]/Message", doc, XPathConstants.NODESET);
								if (eventTypeList != null) {
									for (int k = 1; k <= eventTypeList.getLength(); k++) {
										String user = xpath.evaluate("/Chat/Events/Event[" + i+ "]/Message[" + k + "]/User", doc);
										String utcTime = xpath.evaluate("/Chat/Events/Event[" + i+ "]/Message[" + k + "]/UTCTime", doc);
										String chatContent = xpath.evaluate("/Chat/Events/Event[" + i + "]/Message[" + k + "]/Content", doc)+"content "+i+" "+k;
										if (!chatContent.isEmpty() && !utcTime.isEmpty() && utcTime != null) {
											eventInfo = eventInfo + user_info.get(user.toLowerCase()) + "\n";
											eventInfo = eventInfo + chatContent + "\t";
											String time[] = utcTime.split("T")[1].split(":");
											eventInfo = eventInfo + "(" + time[0] + ":" + time[1] + ")"+ "\n";
										}
									}
								}
							}
						}
						obj.put("eventInfo", eventInfo);
						fs = FileSystem.get(conf);
						FSDataOutputStream out = fs.create(new Path(documentPath + "/" + docname + ".json"), true);
						out.writeBytes(obj.toJSONString());
						out.close();
					}
				});
			}
			JAVA_CONTEXT.close();
		} catch (IOException e1) {
			e1.printStackTrace();
		}

	}
}




//till here 
#include<iostream>

#include<string>
using namespace std;

int main()
 {
	int tc,n,m;
	cin>>tc;
	string s1,s2;
		while(tc>0)
		{
		    cin>>n>>m;
			cin>>s1>>s2;

			cout<<getMinChanges(s1,s2,n-1,m-1);
		    tc--;
		}
	return 0;
}
