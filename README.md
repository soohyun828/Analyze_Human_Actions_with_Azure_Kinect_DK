# allbigdata-kinnect-DX1
## 1. Setup
- Visual Studio 솔루션 탐색기에서 우클릭>>nuget 패키지 관리>>Microsoft.Azure.Kinect.Sensor와 Microsoft.Azure.Kinect.BodyTracking 설치<br><br>

## 객체의 joint 별 x, y, z 좌표 Real-time 모니터링
- ```skeleton.joints.position.cpp```
- 실시간으로 촬영 중인 객체에 대해 joint 별 x, y, z 좌표를 출력함.
- 영상이 저장되거나, 좌표를 따로 저장하지 않음. 단지 viewer처럼 실시간으로 촬영중인 영상의 좌표가 출력되기만 함.<br><br>

## 객체의 joint 별 x, y, z 좌표와 orientation x, y, z좌표 Real-time 모니터링 및 추출
- ```Simple_Sampling.c```
- 실시간으로 촬영 중인 객체에 대해 joint 별 x, y, z 좌표와 orientation x, y, z좌표, confidence level을 총 100 frame까지 출력함.
- 만약 frame 제한을 없애거나 다른 종료조건으로 수정하고 싶다면 github 코드 기준 154행에 있는 while문 조건을 수정해주면 된다.
- 실시간으로 촬영중인 객체에 대한 정보를 뽑아서 출력하고 txt로 저장함. 영상은 따로 저장되지 않음.<br><br>
- 한 객체(사람) 당 32개의 joint tracking<br><br>
  [저장 결과 예시]<br>
  ![Image](https://github.com/user-attachments/assets/7ba1ddf2-3b5d-48de-931f-30662c827e52)

## 사전에 촬영해 .mkv로 저장해둔 영상으로부터 객체의 frame별 joint 좌표 추출
- ```converter.cpp```
- 영상을 Joint 좌표로 변환함 (mkv to txt)<br><br>
  [저장 결과 예시]<br>
  ![Image](https://github.com/user-attachments/assets/c0aa2046-eafc-4690-8604-20190ac1c227)

## 2. Examining Angel Measurements to Classify Correct and Wrong Movements using Neural Networks
- ```Dataset training &testing by MLFNN.ipynb```
- ```joint.data.extraction_iteration.ipynb``` 를 통해서 얻은 팔과 몸통 간의 각도 분석 데이터를 이용하여 NN을 학습시키고, 영상 속 사람의 움직임이 정상적인지, 비정상적인지 검토한다.
- 의료 등 올바른 움직임을 하고 있는지 평가하고 진단해야 하는 산업에서 이용 가능하다. 


# Acknowledgement
This work was supported by Institute of Information & communications Technology Planning & Evaluation (IITP) grant funded by the Korea government(MSIT) (No.RS-2022-00155911, Artificial Intelligence Convergence Innovation Human Resources Development (Kyung Hee University))
