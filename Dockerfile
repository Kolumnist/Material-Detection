FROM continuumio/miniconda3:latest

# Set up a working directory inside the container
# This will be where all your code and data will reside.
WORKDIR /app

# Copy your project files into the container
# The "." refers to the current directory on the host, and "/app" is the target in the container.
COPY . /app

# Install necessary Python dependencies
# Use Conda to create an environment and install dependencies.
# Assuming you have a requirements.txt file for dependencies.
RUN conda install --yes python=3.11

RUN pip freeze > requirements.txt
    
# RUN conda install --yes --file requirements.txt

RUN conda clean --all -f -y

# Install pytorch and stuff


# Add environment variables to support Miniconda
# These ensure Conda works properly inside the container.
ENV PATH /opt/conda/bin:$PATH
ENV PYTHONUNBUFFERED 1

# Expose any necessary ports (optional)
# If your Python application runs a web server or API, expose that port.
EXPOSE 5000

# Set the default command to run your Python script
# Replace `train.py` with the main Python script for your project.
CMD ["python", "hello.py"]